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

#ifndef ATMOTUBE_PLUGIN_IF_H
#define ATMOTUBE_PLUGIN_IF_H

/* Plugin interface */

typedef struct
{
  /* const char* type; */
  const char *filename;
  const char *device_name;
  const char *device_address;
  void *state;
} AtmotubeOutput;

/* Get type of plugin. */
const char *get_plugin_type (void);
int plugin_start (AtmotubeOutput * o);
void temperature (unsigned long ts, unsigned long value);
void humidity (unsigned long ts, unsigned long value);
void voc (unsigned long ts, float value);
int plugin_stop (void);

/* Plugin interface */

#endif /* ATMOTUBE_PLUGIN_IF_H */
