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
#include "interval.h"
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
	PRINT_ERROR("No devices found.\n");
	return ATMOTUBE_RET_ERROR;
    }
    
    ret = atmotube_plugin_find(NULL);
    if (ret == 0) {
	PRINT_ERROR("No plugins found.\n");
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
    }

    return ATMOTUBE_RET_OK;
}

//https://eli.thegreenplace.net/2012/08/24/plugins-in-c

void output_temperature(unsigned long ts, unsigned long value, void* data_ptr)
{
    AtmotubeData* d = (AtmotubeData*)data_ptr;
    AtmotubeOutput* o = d->output;
    //o.temperature(ts, value);
}

void output_humidity(unsigned long ts, unsigned long value, void* data_ptr)
{
    AtmotubeData* d = (AtmotubeData*)data_ptr;
    AtmotubeOutput* o = d->output;
    //o.humidity(ts, value);
}

void output_voc(unsigned long ts, float value, void* data_ptr)
{
    AtmotubeData* d = (AtmotubeData*)data_ptr;
    AtmotubeOutput* o = d->output;
    //o.voc(ts, value);
}
