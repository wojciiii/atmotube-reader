/*
* This file is part of atmotube-reader.
*
* atmotube-reader is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* atmotube-reader is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with atmotube-reader.
* If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <glib.h>
#include <stdbool.h>

#include "atmotube.h"
#include "atmotube-config.h"
#include "interval.h"
#include "output.h"

#define NUM_UUIDS 4

typedef struct
{
  char* name;
  char* deviceAddress;
  char* description;
  gatt_connection_t* connection;
  int resolution;

  bool connected;
  bool registred;
} AtmotubeData;

typedef struct
{
  void* adapter;
  char* search_name;

  GSList* connectableDevices;
  GSList* foundDevices;

} AtmotubeGlData;

static AtmotubeGlData glData;

static char* CHARACTER_UUIDS[NUM_UUIDS] = {
  "db450002-8e9a-4818-add7-6ed94a328ab2",
  "db450003-8e9a-4818-add7-6ed94a328ab2",
  "db450004-8e9a-4818-add7-6ed94a328ab2",
  "db450005-8e9a-4818-add7-6ed94a328ab2"
};

static uuid_t UUIDS[NUM_UUIDS] = { CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0) };

static const char *intervalnames[] = {"VOC", "HUMIDITY", "TEMPERATURE", "STATUS"};

static const char *fmts[] = {INTERVAL_FLOAT, INTERVAL_ULONG, INTERVAL_ULONG, ""};

static uint64_t getTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

static void init_gl_data(AtmotubeGlData *ptr)
{
  ptr->adapter = NULL;
  ptr->search_name = NULL;
  ptr->connectableDevices = NULL;
  ptr->foundDevices = NULL;
}

void atmotube_start()
{
  int i;
  int ret;

  for (i = VOC; i < STATUS; i++)
    {
      ret = gattlib_string_to_uuid(CHARACTER_UUIDS[i], strlen(CHARACTER_UUIDS[i]), &UUIDS[i]);
    if (ret != 0)
      {
        PRINT_DEBUG("gattlib_string_to_uuid failed (ret=%d)\n", ret);
        exit(1);
      }
    }

    DEF_ATMOTUBE_NAME="ATMOTUBE";
    DEF_ATMOTUBE_SEARCH_TIMEOUT = 4;

    init_gl_data(&glData);
 
}

static void atmotube_handle_voc(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 2) {
	PRINT_DEBUG("handle_voc: no data\n");
	return;
    }

    uint16_t voc_input = *(data+1) | ((uint16_t)*(data)) << 8;
    double voc = voc_input / 100.0f;
    PRINT_DEBUG("handle_voc: 0x%x, %f\n", voc_input, voc);

    interval_log(intervalnames[VOC], fmts[VOC], voc);
}

static void atmotube_handle_humidity(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 1) {
	PRINT_DEBUG("handle_humidity: no data\n");
	return;
    }

    uint16_t humidity = data[0] & 0xFF;
    PRINT_DEBUG("handle_humidity: 0x%x, %d%%\n", data[0], humidity);
    interval_log(intervalnames[HUMIDITY], fmts[HUMIDITY], humidity);
}

static void atmotube_handle_temperature(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 1) {
	PRINT_DEBUG("handle_temperature: no data\n");
	return;
    }

    uint16_t temperature = data[0] & 0xFF;
    PRINT_DEBUG("handle_temperature: 0x%x, %d C\n", data[0], temperature);
    interval_log(intervalnames[TEMPERATURE], fmts[TEMPERATURE], temperature);
}

static void atmotube_handle_status(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 1) {
	PRINT_DEBUG("handle_status: no data\n");
	return;
    }

    /* ASDFGHJK                                          */
    /* A  (1b)  - 0 -> 3 sec, 1 -> 30 sec.               */
    /* S  (1b)  - 0 -> calibrating, 1 -> ready.          */
    /* DF (2b)  - reserved.                              */
    /* G  (1b)  - 0 -> not charging, 1 -> charging.      */
    /* HJK (3b) - battery charging level, 25% incremets. */

    const uint8_t first_bit_mask = 0x1;
    const uint8_t mode_offset  = 8;
    const uint8_t calib_offset = 7;
    const uint8_t battery_mask = 0x7;
    const uint8_t charging_offset = 3;

    bool mode = (data[0] >> mode_offset) & first_bit_mask;
    bool calibrating = (data[0] >> calib_offset) & first_bit_mask;
    uint8_t battery_percent = (data[0] & battery_mask) * 25;
    bool charging = (data[0] >> charging_offset) & first_bit_mask;

    printf("Status:\n");

    if (mode) {
	printf("\tmode: slow\n");
    } else {
	printf("\tmode: fast\n");
    }

    if (calibrating) {
	printf("\tdev: calibrating\n");
    } else {
	printf("\tdev: ready\n");
    }

    if (charging) {
	printf("\tpower: charging\n");
    } else {
	printf("\tpower: not charging\n");
    }

    printf("\tbattery: %u%%\n", battery_percent);
}

void atmotube_handle_notification(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
  int i;
  enum CHARACTER_ID id = CHARACTER_MAX;
  AtmotubeData* d = (AtmotubeData*)user_data;

  PRINT_DEBUG("Notification Handler for %s:\n", d->deviceAddress);

  for (i = VOC; i < CHARACTER_MAX; i++)
  {
    if (gattlib_uuid_cmp(uuid, &UUIDS[i]) == 0)
    {
      PRINT_DEBUG("Found id %d\n", i);
      id = (enum CHARACTER_ID)i;
      break;
    }
  }

  switch (id)
  {
    case VOC:
      PRINT_DEBUG("VOC\n");
      atmotube_handle_voc(data, data_length);
      break;
    case HUMIDITY:
      PRINT_DEBUG("HUMIDITY\n");
      atmotube_handle_humidity(data, data_length);
      break;
    case TEMPERATURE:
      PRINT_DEBUG("TEMPERATURE\n");
      atmotube_handle_temperature(data, data_length);
      break;
    case STATUS:
      PRINT_DEBUG("STATUS\n");
      atmotube_handle_status(data, data_length);
      break;
    default:
      PRINT_DEBUG("UNKN %d\n", id);
      break;
  }

  for (i = 0; i < data_length; i++)
  {
    PRINT_DEBUG("%02x ", data[i]);
  }
  PRINT_DEBUG("\n");
}

int atmotube_notify_on_characteristic(gatt_connection_t* connection, enum CHARACTER_ID id)
{
  const char* str_uuid = CHARACTER_UUIDS[id];
  int ret;
  
  PRINT_DEBUG("Register notification for %s.\n", str_uuid);

  ret = gattlib_string_to_uuid(str_uuid, strlen(str_uuid), &UUIDS[id]);
  if (ret != 0)
  {
    PRINT_DEBUG("gattlib_string_to_uuid (ret=%d)\n", ret);
    return 1;
  }

  ret = gattlib_notification_start(connection, &UUIDS[id]);
  if (ret) {
    PRINT_DEBUG("Fail to start notification (ret=%d)\n.", ret);
    return 1;
  }

  return 0;
}

int atmotube_stop_notification(gatt_connection_t* connection, enum CHARACTER_ID id)
{
  const char* str_uuid = CHARACTER_UUIDS[id];
  int ret;
  PRINT_DEBUG("Stop notifications for %s.\n", str_uuid);
  ret = gattlib_notification_stop(connection, &UUIDS[id]);
  if (ret != 0)
  {
    PRINT_DEBUG("Failed to stop notification (ret=%d)\n.", ret);
    return 1;
  }

  return 0;
}

static void discovered_device(const char* addr, const char* name)
{
  PRINT_DEBUG("Compare %s: %s\n", name, glData.search_name);

  if (strcmp(name, glData.search_name) == 0)
  {
    char* temp = malloc(strlen(addr)+1);
    strcpy(temp, addr);
    glData.foundDevices = g_slist_append(glData.foundDevices, temp);
    PRINT_DEBUG("Found atmotube device with name %s: %s.\n", name, addr);
  }
  else
  {
    PRINT_DEBUG("Found other device %s\n", name);
  }
}

int atmotube_search(const char* name, int timeout)
{
  int ret;

  glData.search_name = malloc(strlen(name) + 1);
  strcpy(glData.search_name, name);

  // Using default adapter.
  ret = gattlib_adapter_open(NULL, &glData.adapter);
  if (ret)
  {
    PRINT_DEBUG("gattlib_adapter_open failed.\n");
    return ATMOTUBE_RET_ERROR;
  }

  PRINT_DEBUG("Searching for %s, timeout=%d.\n", name, timeout);
  
  ret = gattlib_adapter_scan_enable(glData.adapter, discovered_device, timeout);
  if (ret)
  {
    PRINT_DEBUG("gattlib_adapter_scan_enable failed.\n");
    return ATMOTUBE_RET_ERROR;
  }

  gattlib_adapter_scan_disable(glData.adapter);
  PRINT_DEBUG("Searching complete\n");
  
  gattlib_adapter_close(glData.adapter);

  return ATMOTUBE_RET_OK;
}

int atmotube_num_found_devices()
{
  return g_slist_length(glData.foundDevices);
}

char** atmotube_get_found_devices()
{
  int const size = atmotube_num_found_devices();
  int buffSize = 0;
  char** output = NULL;
  char** ptr = output;
  GSList *list = NULL;
  int i;

  if (size == 0)
  {
    return NULL;
  }

  output = malloc(size * sizeof(char*));
  ptr = output;
  for (i = 0; i < size; i++)
  {
    list = g_slist_nth (glData.foundDevices, i);
    buffSize = strlen(list->data) + 1;
    *ptr = malloc(buffSize);
    strcpy(*ptr, list->data);
    ptr++;
  }

  return output;
}

int atmotube_add_devices_from_config(char* fullName)
{
  atmotube_config_start(fullName);

  int ret = atmotube_config_load(atmotube_add_device);

  atmotube_config_end();

  return ret;
}

static void dumpAtmotubeData(AtmotubeData* d)
{
  PRINT_DEBUG("d->deviceAddress = %s\n", d->deviceAddress);
}

int atmotube_add_device(char* name, char* deviceAddress, char* description, int resolution)
{
  if (resolution < ATMOTUBE_MIN_RESOUTION)
  {
    PRINT_DEBUG("Resolution is invalid(%d < %d)\n", resolution, ATMOTUBE_MIN_RESOUTION);
    return ATMOTUBE_RET_ERROR;
  }

  if (resolution > ATMOTUBE_MAX_RESOUTION)
  {
    PRINT_DEBUG("Resolution is invalid(%d > %d)\n", resolution, ATMOTUBE_MAX_RESOUTION);
    return ATMOTUBE_RET_ERROR;
  }

  AtmotubeData* d = (AtmotubeData*)malloc(sizeof(AtmotubeData));
  d->name = name;
  d->deviceAddress = strdup(deviceAddress);
  d->description = strdup(description);
  d->connection = NULL;
  d->resolution = resolution;
  d->connected  = 0;
  d->registred  = 0;

  dumpAtmotubeData(d);

  glData.connectableDevices = g_slist_append(glData.connectableDevices, d);

  return ATMOTUBE_RET_OK;
}

static void connect_impl(gpointer data,
                         gpointer user_data)
{
  AtmotubeData* d = (AtmotubeData*)data;
  int* ret = (int*)user_data;

  PRINT_DEBUG("Connecting to %s\n", d->deviceAddress);
  PRINT_DEBUG("Using resolution: %d\n", d->resolution);
  
  d->connection = gattlib_connect(NULL, d->deviceAddress, BDADDR_LE_RANDOM, BT_SEC_LOW, 0, 0);
  //d->connection = gattlib_connect_timeout(NULL, d->deviceAddress, BDADDR_LE_RANDOM, BT_SEC_LOW, 0, 0, 5);
  if (d->connection == NULL)
  {
    PRINT_DEBUG("Failed to connect to the bluetooth device.\n");
    d->connected = false;
    *ret += 1;
  }
  else
  {
    d->connected = true;
    PRINT_DEBUG("Connected\n");
  }
}

static void disconnect_impl(gpointer data,
                            gpointer user_data)
{
  AtmotubeData* d = (AtmotubeData*)data;
  int* ret = (int*)user_data;

  if (d->connected)
  {
    PRINT_DEBUG("Disconnecting %s\n", d->deviceAddress);

    if (gattlib_disconnect(d->connection) == 0)
    {
      PRINT_DEBUG("Disconnected from %s\n", d->deviceAddress);
      d->connected = false;
    }
    else
    {
      PRINT_DEBUG("gattlib_disconnect failed\n");
     *ret += 1;
     d->connected = false;
    }
  }
}

int atmotube_connect()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, connect_impl, &ret);

  if (ret != 0)
  {
    return ATMOTUBE_RET_ERROR;
  }
  else
  {
    return ATMOTUBE_RET_OK;
  }
}

int atmotube_disconnect()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, disconnect_impl, &ret);

  if (ret != 0)
  {
    return ATMOTUBE_RET_ERROR;
  }
  else
  {
    return ATMOTUBE_RET_OK;
  }
}

static void modify_intervals(AtmotubeData* d, bool add_interval)
{
    if (add_interval) {
	PRINT_DEBUG("Adding intervals\n");
    } else {
	PRINT_DEBUG("Removing intervals\n");
    }
    
    uint8_t character_id;
    uint16_t interval = INTERVAL_SEC_TO_MS(d->resolution);
    for (character_id = VOC; character_id < CHARACTER_MAX; character_id++) {
	const char* label = intervalnames[character_id];
	const char* fmt = fmts[character_id];
	if (strlen(fmt) > 0) {
	    
	    if (add_interval) {
		PRINT_DEBUG("Adding interval: %s:%s\n", label, fmt);
		interval_add(label, fmt);
		interval_start(label, fmt, interval);
		
		switch (character_id) {
		case VOC:
		    interval_add_float_callback(label, fmt, output_voc);
		    break;
		case HUMIDITY:
		    interval_add_ulong_callback(label, fmt, output_humidity);
		    break;
		case TEMPERATURE:
		    interval_add_ulong_callback(label, fmt, output_temperature);
		    break;
		case STATUS:
		    break;
		}
	    }
	    else {
		PRINT_DEBUG("Removing interval: %s:%s\n", label, fmt);
		interval_remove_callbacks(label, fmt);
		interval_stop(label, fmt);
		interval_remove(label, fmt);
	    }
	}
    }
}

static void register_impl(gpointer data,
                          gpointer user_data)
{
  AtmotubeData* d = (AtmotubeData*)data;
  int* ud = (int*)user_data;
  int ret = 0;

  if (!d->connected)
  {
      *ud += 1;
      PRINT_DEBUG("Unable to register, not connected to %s\n", d->deviceAddress);
      d->connected = false;
      return;
  }

  PRINT_DEBUG("Register notification\n");
  gattlib_register_notification(d->connection, atmotube_handle_notification, d);
  PRINT_DEBUG("Register notification done\n");

  /* Add intervals. */
  PRINT_DEBUG("Adding intervals\n");
  uint8_t character_id;
  uint16_t interval = INTERVAL_SEC_TO_MS(d->resolution);
  for (character_id = VOC; character_id < CHARACTER_MAX; character_id++) {
      const char* label = intervalnames[character_id];
      const char* fmt = fmts[character_id];
      if (strlen(fmt) > 0) {
	  PRINT_DEBUG("Adding interval: %s:%s\n", label, fmt);
	  interval_add(label, fmt);
	  interval_start(label, fmt, interval);

	  switch (character_id) {
	  case VOC:
	      interval_add_float_callback(label, fmt, output_voc);
	      break;
	  case HUMIDITY:
	      interval_add_ulong_callback(label, fmt, output_humidity);
	      break;
	  case TEMPERATURE:
	      interval_add_ulong_callback(label, fmt, output_temperature);
	      break;
	  case STATUS:
	      break;
	  }
      }
  }

  ret = 0;

  for (character_id = VOC; character_id < CHARACTER_MAX; character_id++) {
      PRINT_DEBUG("Notify on: %u\n", character_id);
      ret += atmotube_notify_on_characteristic(d->connection, character_id);
  }

  if (ret != 0)
  {
    PRINT_DEBUG("atmotube_notify_on_characteristic failed for %s\n", d->deviceAddress);
    gattlib_disconnect(d->connection);
    d->connected = false;
    *ud += 1;
    return;
  }
}

static void unregister_impl(gpointer data,
                            gpointer user_data)
{
    AtmotubeData* d = (AtmotubeData*)data;
    UNUSED(user_data);
    /*int* ud = (int*)user_data;*/
    int ret = 0;

    uint8_t character_id;
    for (character_id = VOC; character_id < CHARACTER_MAX; character_id++) {
	PRINT_DEBUG("Stop notify on: %u\n", character_id);
	ret += atmotube_stop_notification(d->connection, character_id);
    }
}

int atmotube_register()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, register_impl, &ret);

  if (ret != 0)
  {
    return ATMOTUBE_RET_ERROR;
  }
  else
  {
    return ATMOTUBE_RET_OK;
  }
}

int atmotube_unregister()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, unregister_impl, &ret);

  if (ret != 0)
  {
    return ATMOTUBE_RET_ERROR;
  }
  else
  {
    return ATMOTUBE_RET_OK;
  }
}

uuid_t* atmotube_getuuid(enum CHARACTER_ID id)
{
  return &UUIDS[id];
}

void atmotube_end()
{
  atmotube_disconnect();
}
