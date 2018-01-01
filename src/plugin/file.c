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

#include "file.h"
#include <stdbool.h>
#include <stdio.h>
#include <atmotube.h>
#include "atmotube-config.h"

static bool started = false;
static FILE *f = NULL;
static const char *type = "file";

const char *
get_plugin_type (void)
{
  return type;
}

int
plugin_start (AtmotubeOutput * o)
{
  if (o->filename == NULL)
    {
      PRINT_ERROR ("plugin_start, no filename provided\n");
      return ATMOTUBE_RET_ERROR;
    }

  PRINT_DEBUG ("plugin_start, opening file %s for appending\n", o->filename);

  f = fopen (o->filename, "a");
  if (f == NULL)
    {
      PRINT_ERROR ("plugin_start, unable to open file %s for appending\n",
		   o->filename);
      return ATMOTUBE_RET_ERROR;
    }

  started = true;
  return ATMOTUBE_RET_OK;
}

int
plugin_stop (void)
{
  if (started)
    {
      started = false;
      fclose (f);
      return ATMOTUBE_RET_OK;
    }

  PRINT_ERROR ("plugin_stop, invalid state\n");
  return ATMOTUBE_RET_ERROR;
}

void
temperature (unsigned long ts, unsigned long value)
{
  PRINT_DEBUG ("Writing temperature to file(%u): %lu,%lu\n", started, ts,
	       value);

  if (started)
    {
      fprintf (f, "%lu,temperature,%lu\n", ts, value);
      fflush (f);
    }
}

void
humidity (unsigned long ts, unsigned long value)
{
  PRINT_DEBUG ("Writing humidity to file(%u): %lu,%lu\n", started, ts, value);

  if (started)
    {
      fprintf (f, "%lu,humidity,%lu\n", ts, value);
      fflush (f);
    }
}

void
voc (unsigned long ts, float value)
{
  PRINT_DEBUG ("Writing voc to file(%u): %lu,%f\n", started, ts, value);

  if (started)
    {
      fprintf (f, "%lu,voc,%f\n", ts, value);
      fflush (f);
    }
}
