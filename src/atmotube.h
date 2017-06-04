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

#define ATMOTUBE_RET_OK 0
#define ATMOTUBE_RET_ERROR 1

struct stored
{
    uint64_t timestamp;
    float    voc;
    int      temperature;
    int      humidity;
};

// max - max devices
// resolution - resolution is in miliseconds.

void atmotube_start();
void atmotube_end();

// Search for atmotube devices.
int atmotube_search();

// Return the number of found devices.
int atmotube_num_found_devices();

// Get list of found devices.
char** atmotube_get_found_devices();

// Add a device to the list of connectable Atmotube devices.
int atmotube_add_device(char* deviceAddress, int resolution);

// Connect to configured devices.
int atmotube_connect();

void atmotube_handle_notification(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);

int atmotube_notify_on_characteristic(gatt_connection_t* connection, enum CHARACTER_ID id);
int atmotube_stop_notification(gatt_connection_t* connection, enum CHARACTER_ID id);

void atmotube_handle_voc(const uint8_t* data, size_t data_length);
void atmotube_handle_humidity(const uint8_t* data, size_t data_length);
void atmotube_handle_temperature(const uint8_t* data, size_t data_length);
void atmotube_handle_status(const uint8_t* data, size_t data_length);

uuid_t* atmotube_getuuid(enum CHARACTER_ID id);

#endif /* ATMOTUBE_H */
