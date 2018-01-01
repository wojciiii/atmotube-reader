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

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <glib.h>
#include <stdbool.h>

#include "atmotube.h"
#include "atmotube-config.h"
#include "atmotube-interval.h"
#include "atmotube-output.h"
#include "atmotube-private.h"
#include "atmotube-plugin.h"

extern AtmotubeGlData glData;

static void clear_outputs()
{
    int i;
    
    for (i = 0; i < glData.deviceConfigurationSize; i++) {
    AtmotubeData* d = glData.deviceConfiguration + i;
    
    if (d->output != NULL) {
        free(d->output);
        d->output = NULL;
    }
    }
}

int atmotube_create_outputs()
{
    int i;
    int ret;
    PRINT_DEBUG("Devices: %d\n", glData.deviceConfigurationSize);

    if (glData.deviceConfigurationSize == 0) {
    PRINT_ERROR("%s\n", "No devices found.");
    return ATMOTUBE_RET_ERROR;
    }

    PRINT_DEBUG("Using plugin path: %s\n", glData.plugin_path);
    
    ret = atmotube_plugin_find(glData.plugin_path);
    if (ret != ATMOTUBE_RET_OK) {
    PRINT_ERROR("%s\n", "atmotube_create_outputs, no plugins found.");
    return ATMOTUBE_RET_ERROR;
    }

    for (i = 0; i < glData.deviceConfigurationSize; i++) {
    AtmotubeData* d = glData.deviceConfiguration + i;

    d->output = NULL;

    PRINT_DEBUG("Creating output: %s\n", d->device.output_type);
    AtmotubePlugin *op = atmotube_plugin_get(d->device.output_type);
    if (op == NULL) {
        clear_outputs();
        return ATMOTUBE_RET_ERROR;
    }
    d->output = (AtmotubeOutput*)malloc(sizeof(AtmotubeOutput));

    d->output->device_name = d->device.device_name;
    d->output->device_address = d->device.device_address;
        
    d->output->filename = d->device.output_filename;
    d->output->state = NULL;

    d->plugin = op;
    d->plugin->plugin_start(d->output);
    }

    return ATMOTUBE_RET_OK;
}

int atmotube_destroy_outputs()
{
    int i;

    for (i = 0; i < glData.deviceConfigurationSize; i++) {
    AtmotubeData* d = glData.deviceConfiguration + i;

    int status = d->plugin->plugin_stop();
    PRINT_DEBUG("Stopped plugin: %d\n", status);
    free(d->output);
    }
    
    return ATMOTUBE_RET_OK;
}

void output_temperature(unsigned long ts, unsigned long value, void* data_ptr)
{
    AtmotubeData* d = (AtmotubeData*)data_ptr;
    AtmotubePlugin* plugin = d->plugin;
    plugin->temperature(ts, value);
}

void output_humidity(unsigned long ts, unsigned long value, void* data_ptr)
{
    AtmotubeData* d = (AtmotubeData*)data_ptr;
    AtmotubePlugin* plugin = d->plugin;
    plugin->humidity(ts, value);
}

void output_voc(unsigned long ts, float value, void* data_ptr)
{
    AtmotubeData* d = (AtmotubeData*)data_ptr;
    AtmotubePlugin* plugin = d->plugin;
    plugin->voc(ts, value);
}
