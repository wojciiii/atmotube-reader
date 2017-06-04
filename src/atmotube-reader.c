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

#include "atmotube.h"

static int find_atmotube(int timeout, char** found_devices)
{
  /* TODO: implement this. */
  atmotube_end();
  return 1;
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
  // TODO: remove hardcoded address.
    const char* deviceAddress = "F7:35:49:55:35:E5";

  atmotube_start();
    signal(SIGINT, intHandler);
    
  printf("Connecting\n");

    connection = gattlib_connect(NULL, deviceAddress, BDADDR_LE_RANDOM, BT_SEC_LOW, 0, 0);
    if (connection == NULL)
    {
      fprintf(stderr, "Fail to connect to the bluetooth device.\n");
    atmotube_end();
      return 1;
    }

    printf("Connected\n");

  printf("Register notification\n");
  gattlib_register_notification(connection, atmotube_handle_notification, NULL);
  printf("Register notification done\n");

    ret = 0;
  ret += atmotube_notify_on_characteristic(connection, VOC);
    ret += atmotube_notify_on_characteristic(connection, HUMIDITY);
    ret += atmotube_notify_on_characteristic(connection, TEMPERATURE);
    ret += atmotube_notify_on_characteristic(connection, STATUS);

    if (ret != 0)
    {
      gattlib_disconnect(connection);
    atmotube_end();
      return 1;
    }

    loop = g_main_loop_new(NULL, 0);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);

  atmotube_stop_notification(connection, VOC);
    atmotube_stop_notification(connection, HUMIDITY);
    atmotube_stop_notification(connection, TEMPERATURE);
    atmotube_stop_notification(connection, STATUS);

  printf("Disconnecting\n");

    gattlib_disconnect(connection);

    printf("Done\n");

  atmotube_end();
    return 0;
}
