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

#include <signal.h>
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gattlib.h"

#include "atmotube.h"

void intHandler(int dummy)
{
    UNUSED(dummy);
    //printf("INT handler\n");
}

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    int ret;
    const char** devices = NULL;
    const char* device;
    int i;

    atmotube_start();
    signal(SIGINT, intHandler);
    
    ret = atmotube_search(DEF_ATMOTUBE_NAME,
			  DEF_ATMOTUBE_SEARCH_TIMEOUT);
      
    if (ret != ATMOTUBE_RET_OK)
    {
      atmotube_end();
      return ATMOTUBE_RET_ERROR;
    }

    int found = atmotube_num_found_devices();

    if (found == 0)
    {
      printf("No devices found.\n");
    }
    else
    {
      printf("Found %d atmotube device(s):\n", found);
      devices = atmotube_get_found_devices();
      for (i = 0; i < found; i++)
      {
        device = devices[i];
        printf("Atmotube device %d: %s\n", i, device);
      }
    }
    printf("Done\n");

    atmotube_end();
    return ATMOTUBE_RET_OK;
}
