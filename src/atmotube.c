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
#include "atmotube-private.h"

#include "atmotube-config.h"
#include "interval.h"
#include "output.h"

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

static void dumpAtmotubeData(AtmotubeData* d)
{
  PRINT_DEBUG("deviceAddress = %s\n", d->device->device_address);
}

static int atmotibe_add_output(char* src, int device_id, char* filename)
{
    return 0;
}

static int atmotube_add_device(Atmotube_Device* device)
{
  if (device->device_resolution < ATMOTUBE_MIN_RESOUTION)
  {
    PRINT_DEBUG("Resolution is invalid(%d < %d)\n", device->device_resolution, ATMOTUBE_MIN_RESOUTION);
    return ATMOTUBE_RET_ERROR;
  }

  if (device->device_resolution > ATMOTUBE_MAX_RESOUTION)
  {
    PRINT_DEBUG("Resolution is invalid(%d > %d)\n", device->device_resolution, ATMOTUBE_MAX_RESOUTION);
    return ATMOTUBE_RET_ERROR;
  }

  AtmotubeData* d = (AtmotubeData*)malloc(sizeof(AtmotubeData));
  d->device = device;
  
  d->connection = NULL;
  d->connected  = 0;
  d->registred  = 0;

  dumpAtmotubeData(d);

  glData.connectableDevices = g_slist_append(glData.connectableDevices, d);

  return ATMOTUBE_RET_OK;
}

int atmotube_add_devices_from_config(char* fullName)
{
  atmotube_config_start(fullName);

  int ret = atmotube_config_load(atmotube_add_device);

  atmotube_config_end();

  return ret;
}

static void connect_impl(gpointer data,
                         gpointer user_data)
{
  AtmotubeData* d = (AtmotubeData*)data;
  int* ret = (int*)user_data;

  PRINT_DEBUG("Connecting to %s\n", d->device->device_address);
  PRINT_DEBUG("Using resolution: %d\n", d->device->device_resolution);
  
  d->connection = gattlib_connect(NULL, d->device->device_address, BDADDR_LE_RANDOM, BT_SEC_LOW, 0, 0);
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
    PRINT_DEBUG("Disconnecting %s\n", d->device->device_address);

    if (gattlib_disconnect(d->connection) == 0)
    {
      PRINT_DEBUG("Disconnected from %s\n", d->device->device_address);
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
    uint16_t interval = INTERVAL_SEC_TO_MS(d->device->device_resolution);
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
      PRINT_DEBUG("Unable to register, not connected to %s\n", d->device->device_address);
      d->connected = false;
      return;
  }

  PRINT_DEBUG("Register notification\n");
  gattlib_register_notification(d->connection, atmotube_handle_notification, d);
  PRINT_DEBUG("Register notification done\n");

  /* Add intervals. */
  PRINT_DEBUG("Adding intervals\n");
  uint8_t character_id;
  uint16_t interval = INTERVAL_SEC_TO_MS(d->device->device_resolution);
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
    PRINT_DEBUG("atmotube_notify_on_characteristic failed for %s\n", d->device->device_address);
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
