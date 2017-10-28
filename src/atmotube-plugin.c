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

#define FUNCTION_GET_PLUGIN_TYPE "get_plugin_type"
#define FUNCTION_PLUGIN_START "plugin_start"
#define FUNCTION_PLUGIN_STOP "plugin_stop"
#define FUNCTION_TEMPERATURE "temperature"
#define FUNCTION_HUMIDITY "humidity"
#define FUNCTION_VOC "voc"

extern AtmotubeGlData glData;

static GSList* plugins = NULL;

static int numberOfPlugins = 0;

#define LOAD_FUNCTION(ptr,function) *(void **) (&ptr) = dlsym(handle, function)
#define CHECK_DLSYM_RESULT(ptr,function) if (ptr == NULL) { PRINT_ERROR("Unable to load %s\n", function); return ATMOTUBE_RET_ERROR; }

int static plugin_assign(void* handle, AtmotubePlugin *dest) {

    PRINT_DEBUG("plugin_assign\n");
    
    CB_get_plugin_type* get_plugin_type = NULL;
    LOAD_FUNCTION(get_plugin_type, FUNCTION_GET_PLUGIN_TYPE);
    CHECK_DLSYM_RESULT(get_plugin_type,FUNCTION_GET_PLUGIN_TYPE);

    CB_plugin_start* plugin_start = NULL;
    LOAD_FUNCTION(plugin_start, FUNCTION_PLUGIN_START);
    CHECK_DLSYM_RESULT(plugin_start,FUNCTION_PLUGIN_START);

    CB_temperature* temperature = NULL;
    LOAD_FUNCTION(temperature, FUNCTION_TEMPERATURE);
    CHECK_DLSYM_RESULT(temperature,FUNCTION_TEMPERATURE);

    CB_humidity* humidity = NULL;
    LOAD_FUNCTION(humidity, FUNCTION_HUMIDITY);
    CHECK_DLSYM_RESULT(humidity,FUNCTION_HUMIDITY);

    CB_voc* voc = NULL;
    LOAD_FUNCTION(voc, FUNCTION_VOC);
    CHECK_DLSYM_RESULT(voc, FUNCTION_VOC);

    CB_plugin_stop* plugin_stop = NULL;
    LOAD_FUNCTION(plugin_stop, FUNCTION_PLUGIN_STOP);
    CHECK_DLSYM_RESULT(plugin_stop,FUNCTION_PLUGIN_STOP);

    dest->get_plugin_type = get_plugin_type;
    dest->plugin_start = plugin_start;
    dest->temperature  = temperature;
    dest->humidity     = humidity;
    dest->voc          = voc;
    dest->plugin_stop  = plugin_stop;

    PRINT_DEBUG("All functions present\n");
    
    return ATMOTUBE_RET_OK;
}

int atmotube_plugin_find(const char* path)
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

	    AtmotubePlugin *info = malloc(sizeof(AtmotubePlugin));
	    if (plugin_assign(libhandle, info) != ATMOTUBE_RET_OK) {
		dlclose(info->handle);
		free(info);
		info = NULL;
		continue;
	    }

	    info->handle = libhandle;
	    PRINT_DEBUG("Found function call: %s\n", FUNCTION_GET_PLUGIN_TYPE);
	    info->type = strdup(info->get_plugin_type());
	    PRINT_DEBUG("Found plugin type: %s\n", info->type);

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

AtmotubePlugin* atmotube_plugin_get(const char* type)
{
    int i;
    GSList *node;

    if (numberOfPlugins == 0) {
	PRINT_ERROR("No plugins loaded.\n");
	return NULL;
    }
    
    for (i = 0; i < numberOfPlugins; i++) {
	PRINT_DEBUG("Plugin %d: \n", i);
	node = g_slist_nth(plugins, i);
	AtmotubePlugin *info = (AtmotubePlugin*)node->data;
	if (strcmp(info->type, type) == 0) {
	    return info;
	}
    }

    PRINT_ERROR("Plugin with type '%s' not found\n", type);
    return NULL;
}

int atmotube_plugin_unload_all()
{
    int i;
    GSList *node;

    for (i = 0; i < numberOfPlugins; i++) {
	node = g_slist_nth(plugins, i);
	AtmotubePlugin *info = (AtmotubePlugin*)node->data;
	dlclose(info->handle);
	free(info);
    }

    g_slist_free(plugins);
    numberOfPlugins = 0;
    plugins = NULL;
    
    return ATMOTUBE_RET_ERROR;
}

