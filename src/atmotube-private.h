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

#ifndef ATMOTUBE_PRIVATE_H
#define ATMOTUBE_PRIVATE_H

#include "atmotube.h"
#include "atmotube-config.h"
#include "atmotube-output.h"
#include "atmotube-plugin-if.h"
#include "atmotube-plugin.h"
#include "atmotube-interval.h"

#include <stdbool.h>
#include <glib.h>

#define NUM_UUIDS 4

/* Global data used internally. */

/* TODO: rename this! */
typedef struct
{
    /* Description from config:*/
    Atmotube_Device device;
    /* Runtime settings: */
    gatt_connection_t* connection;
    bool connected;
    /* bool registred; */

    AtmotubeOutput* output;
    AtmotubePlugin* plugin;
} AtmotubeData;

typedef struct
{
    void* adapter;
    char* search_name;

    GSList* connectableDevices;
    GSList* foundDevices;

    /* Number of devices read from configuration. */
    int deviceConfigurationSize;
    /* List of pointers to device configurations. */
    AtmotubeData* deviceConfiguration;

    const char* plugin_path;
} AtmotubeGlData;

extern AtmotubeGlData glData;

extern char* CHARACTER_UUIDS[NUM_UUIDS];
extern uuid_t UUIDS[NUM_UUIDS];
extern char *intervalnames[NUM_UUIDS];
extern char *fmts[NUM_UUIDS];

uuid_t* atmotube_getuuid(enum CHARACTER_ID id);

#endif /* ATMOTUBE_PRIVATE_H */
