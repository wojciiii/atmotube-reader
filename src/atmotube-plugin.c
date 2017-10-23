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
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "atmotube-plugin.h"
#include "atmotube.h"
#include "atmotube-config.h"
#include "interval.h"
#include "atmotube-private.h"

#define GET_PLUGIN_TYPE "get_plugin_type"

extern AtmotubeGlData glData;

typedef struct
{
    int type;
    void* handle;
} PluginInfo;

static GSList* plugins = NULL;

static int numberOfPlugins = 0;

int plugin_find(char* path)
{
    if (path == NULL) {
	// TODO: implement this.
	return ATMOTUBE_RET_ERROR;
    }
    
    PRINT_DEBUG("Searching for DSO(s) in %s\n", path);

    DIR* dir = opendir(path);
    if (!dir) {
	PRINT_ERROR("Unable to read path: %s\n", path);
	return ATMOTUBE_RET_ERROR;
    }

    struct dirent* direntry;

    while ((direntry = readdir(dir)) != NULL) {
	char *filename = direntry->d_name;
	char *dot = strrchr(filename, '.');
	if (dot && !strcmp(dot, ".so")) {
	    const size_t len = strlen(path) + strlen("/") + strlen(filename) + 1;
	    char fullname[len];
	    snprintf(&fullname[0], sizeof(fullname), "%s/%s", path, filename);

	    void* libhandle = dlopen(&fullname[0], RTLD_NOW);
	    if (libhandle == NULL) {
		PRINT_ERROR("Unable to use plugin: %s\n", &fullname[0]);
		continue;
	    }

	    PRINT_DEBUG("Loading plugin: %s\n", fullname);
	    
	    int (*get_plugin_type)(void) = NULL;

	    *(void **) (&get_plugin_type) = dlsym(libhandle, GET_PLUGIN_TYPE);

	    if (get_plugin_type == NULL) {
		continue;
	    }
	    
	    PRINT_DEBUG("Found function call: %s\n", GET_PLUGIN_TYPE);
	    int type = get_plugin_type();
	    PRINT_DEBUG("Found plugin type: %d\n", type);

	    PluginInfo *info = malloc(sizeof(PluginInfo));
	    info->handle = libhandle;
	    info->type = type;

	    plugins = g_slist_append(plugins, info);

	    numberOfPlugins++;
	}
    }

    closedir(dir);

    if (numberOfPlugins > 0) {
	return ATMOTUBE_RET_OK;
    }
    
    return ATMOTUBE_RET_ERROR;
}

int plugin_load(char* path, int type)
{
    return ATMOTUBE_RET_ERROR;
}

AtmotubePlugin* plugin_get(int type)
{
    return NULL;
}

int plugin_unload_all()
{
    return ATMOTUBE_RET_ERROR;
}

