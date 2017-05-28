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

#include <signal.h>
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "gattlib.h"

/*
  Taken from: https://atmotube.com/api.html:
  
  db450001-8e9a-4818-add7-6ed94a328ab2 Atmotube service UUID
  db450002-8e9a-4818-add7-6ed94a328ab2 VOC characteristic
  db450003-8e9a-4818-add7-6ed94a328ab2 Humidity characteristic
  db450004-8e9a-4818-add7-6ed94a328ab2 Temperature characteristic
  db450005-8e9a-4818-add7-6ed94a328ab2 Status characteristic
*/	  
enum CHARACTER_ID
{
  VOC = 0,
  HUMIDITY,
  TEMPERATURE,
  STATUS,
  CHARACTER_MAX
};

int CHARACTER_IDS[] = { HUMIDITY, TEMPERATURE, STATUS };

const char* CHARACTER_UUIDS[] = {
  "db450002-8e9a-4818-add7-6ed94a328ab2",
  "db450003-8e9a-4818-add7-6ed94a328ab2",
  "db450004-8e9a-4818-add7-6ed94a328ab2",
  "db450005-8e9a-4818-add7-6ed94a328ab2"
};

uuid_t UUIDS[] = { CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0) };

// Battery Level UUID
const uuid_t g_battery_level_uuid = CREATE_UUID16(0x2A19);

void notification_handler(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
  int i;
  enum CHARACTER_ID id = CHARACTER_MAX;

  printf("Notification Handler: ");

  for (i = VOC; i < STATUS; i++)
  {
    if (gattlib_uuid_cmp(uuid, &UUIDS[i]) == 0)
    {
      printf("Found id %d\n", i);
      id = (enum CHARACTER_ID)i;
      break;
    }
  }

  switch (id)
  {
    case VOC:
      printf("VOC\n");
      break;
    case HUMIDITY:
      printf("HUMIDITY\n");
      break;
    case TEMPERATURE:
      printf("TEMPERATURE\n");
      break;
    case STATUS:
      printf("STATUS\n");
      break;
    default:
      printf("UNKN %d\n", id);
      break;
  }

  for (i = 0; i < data_length; i++)
  {
    printf("%02x ", data[i]);
  }
  printf("\n");
}

static int find_atmotube(int timeout, char** found_devices)
{
  return 1;
}

static int notify_on_characteristic(gatt_connection_t* connection, enum CHARACTER_ID id)
{
  const char* str_uuid = CHARACTER_UUIDS[id];
  uuid_t uuid;
  int ret;
  
  printf("Register notification for %s.\n", str_uuid);
  //gattlib_register_notification(connection, notification_handler, NULL);

  ret = gattlib_string_to_uuid(str_uuid, strlen(str_uuid), &UUIDS[id]);
  if (ret != 0)
  {
    printf("gattlib_string_to_uuid, ret=%d\n", ret);
    return 1;
  }

  ret = gattlib_notification_start(connection, &UUIDS[id]);
  if (ret) {
    fprintf(stderr, "Fail to start notification\n.");
    return 1;
  }

  return 0;
}

static int stop_notification(gatt_connection_t* connection, enum CHARACTER_ID id)
{
  const char* str_uuid = CHARACTER_UUIDS[id];
  int ret;
  printf("Stop notifications for %s.\n", str_uuid);
  ret = gattlib_notification_stop(connection, &UUIDS[id]);
  if (ret != 0)
  {
    fprintf(stderr, "Fail to stop notification\n.");
    return 1;
  }

  return 0;
}

static GMainLoop *loop = NULL;

void intHandler(int dummy)
{
  printf("INT handler\n");
  
  if (loop != NULL)
  {
    g_main_loop_quit (loop);
  }
}

int main(int argc, char *argv[]) {
	int ret;
	gatt_connection_t* connection;
	char* deviceAddress = "F7:35:49:55:35:E5";
	//uuid_t service_uuid;
	//char* service_uuid_str = "db450002-8e9a-4818-add7-6ed94a328ab2";
	//ret = gattlib_string_to_uuid(service_uuid_str, strlen(service_uuid_str), &service_uuid);
	//if (ret != 0)
	//{
	//  printf("gattlib_string_to_uuid, ret=%d\n", ret);
	//  return 0;
	//}

	signal(SIGINT, intHandler);
	
  printf("Connecting\n");

	connection = gattlib_connect(NULL, deviceAddress, BDADDR_LE_RANDOM, BT_SEC_LOW, 0, 0);
	if (connection == NULL)
	{
	  fprintf(stderr, "Fail to connect to the bluetooth device.\n");
	  return 1;
	}

	printf("Connected\n");

  printf("Register notification\n");
  gattlib_register_notification(connection, notification_handler, NULL);
  printf("Register notification done\n");

	ret = 0;
  ret += notify_on_characteristic(connection, VOC);
	ret += notify_on_characteristic(connection, HUMIDITY);
	ret += notify_on_characteristic(connection, TEMPERATURE);
	ret += notify_on_characteristic(connection, STATUS);

	if (ret != 0)
	{
	  gattlib_disconnect(connection);
	  return 1;
	}

/*
	ret = gattlib_notification_start(connection, &service_uuid);
	if (ret) {
		fprintf(stderr, "Fail to start notification\n.");
		return 1;
	}
*/

	loop = g_main_loop_new(NULL, 0);
	g_main_loop_run(loop);

	g_main_loop_unref(loop);

  stop_notification(connection, VOC);
	stop_notification(connection, HUMIDITY);
	stop_notification(connection, TEMPERATURE);
	stop_notification(connection, STATUS);

  printf("Disconnecting\n");

	gattlib_disconnect(connection);

	printf("Done\n");
	return 0;
}
