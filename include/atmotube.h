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

#ifndef ATMOTUBE_H
#define ATMOTUBE_H

#include <gattlib.h>

#define ATMOTUBE_RET_OK 0
#define ATMOTUBE_RET_ERROR 1

#ifndef DEBUG
#define DEBUG 1
#endif

#if (DEBUG)
#include <stdio.h>
#  define PRINT_DEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#  define PRINT_DEBUG(fmt, ...)
#endif

#define PRINT_ERROR(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define UNUSED(x) (void)(x)

enum CHARACTER_ID
{
  VOC = 0,
  HUMIDITY,
  TEMPERATURE,
  STATUS,
  CHARACTER_MAX
};

#define ATMOTUBE_MIN_RESOUTION 100     /* ms */
#define ATMOTUBE_MAX_RESOUTION 60*1000 /* ms */
#define ATMOTUBE_DEF_RESOUTION 1000    /* ms */

struct stored
{
    uint64_t timestamp;
    float    voc;
    int      temperature;
    int      humidity;
};

extern const char* DEF_ATMOTUBE_NAME;
extern int DEF_ATMOTUBE_SEARCH_TIMEOUT;

// max - max devices
// resolution - resolution is in miliseconds.

void atmotube_start();
void atmotube_end();

// Search for atmotube devices.
int atmotube_search(const char* name, int timeout);

// Return the number of found devices.
int atmotube_num_found_devices();

// Get list of found devices.
const char** atmotube_get_found_devices();

// Read devices from a config file.
int atmotube_add_devices_from_config(const char* fullName);

/* Create output plugins described by config. */
int atmotube_create_outputs();

/* Find atmotube plugins. */
int atmotube_plugin_find(const char* path);

// Add a device to the list of connectable Atmotube devices.
//int atmotube_add_device(char* name, char* deviceAddress, char* description, int resolution);

// Connect to configured devices.
int atmotube_connect();

int atmotube_register();
int atmotube_unregister();

// Disconnect from configured devices.
int atmotube_disconnect();

#endif /* ATMOTUBE_H */
