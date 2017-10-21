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
#include "interval.h"

#define NUM_UUIDS 4

typedef struct
{
    /* Description from config:*/
    Atmotube_Device device;
    /* Runtime settings: */
    gatt_connection_t* connection;
    bool connected;
    bool registred;
} AtmotubeData;

typedef struct
{
  void* adapter;
  char* search_name;

  GSList* connectableDevices;
  GSList* foundDevices;

} AtmotubeGlData;

static AtmotubeGlData glData;

static char* CHARACTER_UUIDS[NUM_UUIDS] = {
  "db450002-8e9a-4818-add7-6ed94a328ab2",
  "db450003-8e9a-4818-add7-6ed94a328ab2",
  "db450004-8e9a-4818-add7-6ed94a328ab2",
  "db450005-8e9a-4818-add7-6ed94a328ab2"
};

static uuid_t UUIDS[NUM_UUIDS] = { CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0), CREATE_UUID16(0x0) };

static const char *intervalnames[] = {"VOC", "HUMIDITY", "TEMPERATURE", "STATUS"};

static const char *fmts[] = {INTERVAL_FLOAT, INTERVAL_ULONG, INTERVAL_ULONG, ""};

#endif
