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

#ifndef ATMOTUBE_CONFIG_H
#define ATMOTUBE_CONFIG_H

#include <stddef.h>

#define OUTPUT_FILE 0x1
#define OUTPUT_DB   0x2
/* Some other plugin, not implemented yet: */
#define OUTPUT_CUSTOM 0x3

typedef struct Atmotube_Device_S {
    /* Device: */
    int device_id;
    const char* device_name;
    const char* device_address;
    const char* device_description;
    int device_resolution;

    /* Output: */
    int output_type;
    const char* output_filename;
} Atmotube_Device;

void atmotube_config_start(char* fullName);

typedef int (deviceCB)(void* memory);
typedef void* (NumDevicesCB)(int numDevices);

/* offset - Atmotube_Device offset in the provided memory which is (n x element_size) size bytes. */
int atmotube_config_load(NumDevicesCB numDevicesCb, deviceCB deviceCb, size_t element_size, size_t offset);

void atmotube_config_end();

#endif /* ATMOTUBE_CONFIG_H */
