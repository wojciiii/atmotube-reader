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

#include "atmotube.h"
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUM_UUIDS 4

#ifndef DEBUG
#define DEBUG 1
#endif

#if (DEBUG)
#  define PRINT_DEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#  define PRINT_DEBUG(fmt, ...)
#endif

static char* CHARACTER_UUIDS[NUM_UUIDS] = {
  "db450002-8e9a-4818-add7-6ed94a328ab2",
  "db450003-8e9a-4818-add7-6ed94a328ab2",
  "db450004-8e9a-4818-add7-6ed94a328ab2",
  "db450005-8e9a-4818-add7-6ed94a328ab2"
};

static uuid_t UUIDS[NUM_UUIDS] = { CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0) };

static uint64_t getTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void atmotube_handle_voc(const uint8_t* data, size_t data_length)
{
	uint16_t voc_input = *(data+1) | ((uint16_t)*(data)) << 8;
	double voc = voc_input / 100.0f;
	PRINT_DEBUG("handle_voc: %ld, %f\n", voc_input, voc);
}

void atmotube_handle_humidity(const uint8_t* data, size_t data_length)
{

}

void atmotube_handle_temperature(const uint8_t* data, size_t data_length)
{

}

void atmotube_handle_status(const uint8_t* data, size_t data_length)
{

}

void atmotube_handle_notification(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
  int i;
  enum CHARACTER_ID id = CHARACTER_MAX;

  PRINT_DEBUG("Notification Handler: \n");

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
  uuid_t uuid;
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
}

void atmotube_end()
{

}

void atmotube_add_device(struct atmotube_data* atmotube)
{
	
}

uuid_t* atmotube_getuuid(enum CHARACTER_ID id)
{
	return &UUIDS[id];
}