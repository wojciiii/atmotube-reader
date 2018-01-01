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
#include "atmotube-private.h"

static bool present(const char* name)
{
    GSList *iter;
    for (iter = glData.foundDevices; iter; iter = iter->next) {
    char* elemName = (char*)iter->data;
    if (strcmp(name, elemName) == 0) {
        return true;
    }
    }
    return false;
}

static void discovered_device(const char* addr, const char* name)
{
    // PRINT_DEBUG("Compare %s: %s\n", name, glData.search_name);

    if (name == NULL) {
    return;
    }

    if ((strcmp(name, glData.search_name) == 0) && (!present(addr))) {
    char* temp = malloc(strlen(addr)+1);
    strcpy(temp, addr);
    glData.foundDevices = g_slist_append(glData.foundDevices, temp);
    PRINT_DEBUG("Found atmotube device with name %s: %s.\n", name, addr);
    /*
    } else {
    PRINT_DEBUG("Found other device %s\n", name);
    */
    }
}

int atmotube_search(const char* name, int timeout)
{
  int ret;

  glData.search_name = malloc(strlen(name) + 1);
  strcpy(glData.search_name, name);

  // Using default adapter.
  ret = gattlib_adapter_open(NULL, &glData.adapter);
  if (ret) {
      PRINT_DEBUG("gattlib_adapter_open failed.\n");
      return ATMOTUBE_RET_ERROR;
  }

  PRINT_DEBUG("Searching for %s, timeout=%d.\n", name, timeout);
  
  ret = gattlib_adapter_scan_enable(glData.adapter, discovered_device, timeout);
  if (ret) {
      PRINT_DEBUG("gattlib_adapter_scan_enable failed.\n");
      return ATMOTUBE_RET_ERROR;
  }

  gattlib_adapter_scan_disable(glData.adapter);
  PRINT_DEBUG("Searching complete\n");
  
  gattlib_adapter_close(glData.adapter);

  return ATMOTUBE_RET_OK;
}

int atmotube_num_found_devices()
{
  return g_slist_length(glData.foundDevices);
}

const char** atmotube_get_found_devices()
{
  int const size = atmotube_num_found_devices();
  int buffSize = 0;
  char** output = NULL;
  char** ptr = output;
  GSList *list = NULL;
  int i;

  if (size == 0) {
      return NULL;
  }

  glData.found_devices_output = malloc(size * sizeof(char*));
  ptr = glData.found_devices_output;
  for (i = 0; i < size; i++) {
      list = g_slist_nth (glData.foundDevices, i);
      buffSize = strlen(list->data) + 1;
      *ptr = malloc(buffSize);
      strcpy(*ptr, list->data);
      ptr++;
  }

  return (const char**)glData.found_devices_output;
}

