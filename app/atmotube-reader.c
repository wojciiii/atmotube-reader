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

#include <atmotube.h>

static GMainLoop *loop = NULL;
static bool aborted    = false;


void intHandler(int dummy)
{
    printf("INT handler\n");
    
    if (loop != NULL) {
	g_main_loop_quit (loop);
    }
    aborted = true;
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
    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to add devices from config.\n");
        atmotube_end();
	return 1;
    }

    ret = atmotube_create_outputs();
    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to create output(s).\n");
        atmotube_end();
	return 1;
    }

    int retry             = 0;
    int max_retries       = 10;
    bool connected        = false;
    uint16_t start        = 500;
    uint16_t milliseconds = 500;
    uint16_t increase     = 250;

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

	if (aborted) {
	    break;
	}
	
	retry++;
    }

    if (aborted) {
	printf("Aborted (signal handler).\n");
	atmotube_end();
	return 1;
    }
    
    if (!connected) {
	printf("Failed to connect to device. Giving up.\n");
	atmotube_end();
	return 1;
    }

    printf("Registering handlers.\n");
    ret = atmotube_register();
    if (ret != ATMOTUBE_RET_OK) {
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
}
