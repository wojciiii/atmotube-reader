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

#ifndef ATMOTUBE_PLUGIN_H
#define ATMOTUBE_PLUGIN_H

#include "atmotube-output.h"
#include "atmotube-plugin-if.h"

typedef const char *(CB_get_plugin_type) (void);
typedef int (CB_plugin_start) (AtmotubeOutput * o);
typedef int (CB_temperature) (unsigned long ts, unsigned long value);
typedef int (CB_humidity) (unsigned long ts, unsigned long value);
typedef int (CB_voc) (unsigned long ts, float value);
typedef int (CB_plugin_stop) (void);

typedef struct
{
  const char *type;
  void *handle;

  CB_get_plugin_type *get_plugin_type;
  CB_plugin_start *plugin_start;
  CB_temperature *temperature;
  CB_humidity *humidity;
  CB_voc *voc;
  CB_plugin_stop *plugin_stop;
} AtmotubePlugin;

/* Finding / loading */
AtmotubePlugin *atmotube_plugin_get (const char *type);

int atmotube_plugin_unload_all ();

#endif /* ATMOTUBE_PLUGIN_H */
