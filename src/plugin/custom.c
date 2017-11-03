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

#include "custom.h"
#include <stdbool.h>
#include <stdio.h>
#include <atmotube.h>
#include "atmotube-config.h"

static bool started = false;
static FILE* f = NULL;
static const char* type = "custom";
    
const char* get_plugin_type(void)
{
    return type;
}

int plugin_start(AtmotubeOutput* o)
{
    f = fopen(o->filename, "a");
    if (f == NULL) {
	PRINT_ERROR("Unable to open file %s for appending", o->filename);
	return ATMOTUBE_RET_ERROR;
    }
    
    started = true;
    return -1;
}

int plugin_stop(void)
{
    if (started) {
	started = false;
	fclose(f);
	return ATMOTUBE_RET_OK;
    }

    PRINT_ERROR("Invalid state");
    return ATMOTUBE_RET_ERROR;
}

void temperature(unsigned long ts, unsigned long value)
{
    if (started) {
	fprintf(f, "%lu,temperature,%lu", ts, value);
    }
}

void humidity(unsigned long ts, unsigned long value)
{
    if (started) {
	fprintf(f, "%lu,humidity,%lu", ts, value);
    }
}

void voc(unsigned long ts, float value)
{
    if (started) {
	fprintf(f, "%lu,voc,%f", ts, value);
    }
}
