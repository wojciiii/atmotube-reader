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
#include <stdbool.h>

#include "gattlib.h"

#include "atmotube.h"

static GMainLoop *loop = NULL;

void intHandler(int dummy)
{
  printf("INT handler\n");
  
  if (loop != NULL)
  {
    g_main_loop_quit (loop);
  }
}

/* Sleep for a number of milliseconds. */
static int sleep_ms(uint16_t milliseconds)
{
    struct timespec ts;
    printf("Sleeping for %d ms.\n", milliseconds);
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    return nanosleep(&ts, NULL);
}

int main(int argc, char *argv[]) {
    int ret;

    atmotube_start();
    signal(SIGINT, intHandler);
    
    ret = atmotube_add_devices_from_config(NULL);
    int retry             = 0;
    int max_retries       = 10;
    bool connected        = false;
    uint16_t start        = 500;
    uint16_t milliseconds = 500;
    uint16_t increase     = 250;
    bool aborted          = false;

    while (retry < max_retries) {
	printf("Connecting to device (%d/%d).\n", retry, max_retries);
	ret = atmotube_connect();
	if (ret == ATMOTUBE_RET_OK) {
	    connected = true;
	    break;
	}
	milliseconds = start + (retry * increase);
	if (sleep_ms(milliseconds) != 0) {
	    aborted = true;
	    connected = false;
	    break;
	}

	retry++;
    }

    if (aborted) {
	printf("Aborted (signal handler).\n");
	atmotube_end();
	return 0;
    }
    
    if (!connected) {
	printf("Failed to connect to device. Giving up.\n");
	atmotube_end();
	return 0;
    }

    printf("Registering handlers.\n");
    ret = atmotube_register();
    if (ret != ATMOTUBE_RET_OK)
    {
        atmotube_disconnect();
        atmotube_end();
    }

    printf("Ready to receive.\n");
    
    loop = g_main_loop_new(NULL, 0);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    atmotube_unregister();
    atmotube_disconnect();
    atmotube_end();

    return 0;

/*
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
    */
}
