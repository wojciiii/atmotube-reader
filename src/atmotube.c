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
#include "atmotube-config.h"
#include "atmotube-interval.h"
#include "atmotube-output.h"
#include "atmotube-handler.h"

AtmotubeGlData glData;

char *CHARACTER_UUIDS[NUM_UUIDS] = {
  "db450002-8e9a-4818-add7-6ed94a328ab2",
  "db450003-8e9a-4818-add7-6ed94a328ab2",
  "db450004-8e9a-4818-add7-6ed94a328ab2",
  "db450005-8e9a-4818-add7-6ed94a328ab2"
};

uuid_t UUIDS[NUM_UUIDS] =
  { CREATE_UUID16 (0x0), CREATE_UUID16 (0x0), CREATE_UUID16 (0x0),
  CREATE_UUID16 (0x0)
};
char *intervalnames[] = { "VOC", "HUMIDITY", "TEMPERATURE", "STATUS" };
char *fmts[] = { INTERVAL_FLOAT, INTERVAL_ULONG, INTERVAL_ULONG, "" };

const char *DEF_ATMOTUBE_NAME = "ATMOTUBE";
int DEF_ATMOTUBE_SEARCH_TIMEOUT = 10;

static void
init_gl_data (AtmotubeGlData * ptr)
{
  ptr->adapter = NULL;
  ptr->search_name = NULL;
  ptr->connectableDevices = NULL;
  ptr->foundDevices = NULL;
  ptr->found_devices_output = NULL;

  ptr->deviceConfigurationSize = 0;
  ptr->deviceConfiguration = NULL;
  ptr->plugin_path = NULL;
}

void
atmotube_start ()
{
  int i;
  int ret;

  for (i = VOC; i < STATUS; i++)
    {
      ret =
	gattlib_string_to_uuid (CHARACTER_UUIDS[i],
				strlen (CHARACTER_UUIDS[i]), &UUIDS[i]);
      if (ret != 0)
	{
	  PRINT_DEBUG ("gattlib_string_to_uuid failed (ret=%d)\n", ret);
	  exit (1);
	}
    }
  init_gl_data (&glData);
}

int
atmotube_notify_on_characteristic (gatt_connection_t * connection,
				   enum CHARACTER_ID id)
{
  const char *str_uuid = CHARACTER_UUIDS[id];
  int ret;

  PRINT_DEBUG ("Register notification for %s.\n", str_uuid);

  ret = gattlib_string_to_uuid (str_uuid, strlen (str_uuid), &UUIDS[id]);
  if (ret != 0)
    {
      PRINT_DEBUG ("gattlib_string_to_uuid (ret=%d)\n", ret);
      return 1;
    }

  ret = gattlib_notification_start (connection, &UUIDS[id]);
  if (ret)
    {
      PRINT_DEBUG ("Fail to start notification (ret=%d)\n.", ret);
      return 1;
    }

  return 0;
}

int
atmotube_stop_notification (gatt_connection_t * connection,
			    enum CHARACTER_ID id)
{
  const char *str_uuid = CHARACTER_UUIDS[id];
  int ret;
  PRINT_DEBUG ("Stop notifications for %s.\n", str_uuid);
  ret = gattlib_notification_stop (connection, &UUIDS[id]);
  if (ret != 0)
    {
      PRINT_DEBUG ("Failed to stop notification (ret=%d)\n.", ret);
      return 1;
    }

  return 0;
}

static void
dumpAtmotubeData (AtmotubeData * d)
{
  PRINT_DEBUG ("DUMP: deviceAddress = %s\n", d->device.device_address);
}

/* Get a pointer to list of structs used for devices. */
static void *
atmotube_num_devices (int n)
{
  glData.deviceConfigurationSize = n;
  glData.deviceConfiguration =
    (AtmotubeData *) malloc (sizeof (AtmotubeData) * n);
  return glData.deviceConfiguration;
}

static int
atmotube_add_device (void *m)
{
  Atmotube_Device *d = (Atmotube_Device *) m;

  if (d->device_resolution < ATMOTUBE_MIN_RESOUTION)
    {
      PRINT_DEBUG ("Resolution is invalid(%d < %d)\n", d->device_resolution,
		   ATMOTUBE_MIN_RESOUTION);
      return ATMOTUBE_RET_ERROR;
    }

  if (d->device_resolution > ATMOTUBE_MAX_RESOUTION)
    {
      PRINT_DEBUG ("Resolution is invalid(%d > %d)\n", d->device_resolution,
		   ATMOTUBE_MAX_RESOUTION);
      return ATMOTUBE_RET_ERROR;
    }

  return ATMOTUBE_RET_OK;
}

static void
atmotube_set_plugin_path (const char *path)
{
  PRINT_DEBUG ("Using plugin path: %s\n", path);
  glData.plugin_path = strdup (path);
}

int
atmotube_add_devices_from_config (const char *fullName)
{
  atmotube_config_start (fullName);
  int ret = atmotube_config_load (atmotube_set_plugin_path,
				  atmotube_num_devices,
				  atmotube_add_device, sizeof (AtmotubeData),
				  offsetof (AtmotubeData, device));

  if (ret != ATMOTUBE_RET_OK)
    {
      /* Deallocate any memory. */
      atmotube_config_end ();
      free (glData.deviceConfiguration);
      return ATMOTUBE_RET_ERROR;
    }

  PRINT_DEBUG ("Devices: %d\n", glData.deviceConfigurationSize);

  int i = 0;
  for (i = 0; i < glData.deviceConfigurationSize; i++)
    {
      AtmotubeData *d = glData.deviceConfiguration + i;
      d->connection = NULL;
      d->connected = 0;
      //d->registred  = 0;
      d->output = NULL;
      d->plugin = NULL;
      dumpAtmotubeData (d);
      glData.connectableDevices =
	g_slist_append (glData.connectableDevices, d);
    }

  atmotube_config_end ();

  return ret;
}

static void
connect_impl (gpointer data, gpointer user_data)
{
  AtmotubeData *d = (AtmotubeData *) data;
  int *ret = (int *) user_data;

  if (d->connected)
    {
      PRINT_DEBUG ("Already connected to %s\n", d->device.device_address);
      return;
    }

  PRINT_DEBUG ("Connecting to %s\n", d->device.device_address);
  PRINT_DEBUG ("Using resolution: %d\n", d->device.device_resolution);

  d->connection =
    gattlib_connect (NULL, d->device.device_address, BDADDR_LE_RANDOM,
		     BT_SEC_LOW, 0, 0);
  //d->connection = gattlib_connect_timeout(NULL, d->deviceAddress, BDADDR_LE_RANDOM, BT_SEC_LOW, 0, 0, 5);
  if (d->connection == NULL)
    {
      PRINT_DEBUG ("%s\n", "Failed to connect to the bluetooth device.");
      d->connected = false;
      *ret += 1;
    }
  else
    {
      d->connected = true;
      PRINT_DEBUG ("%s\n", "Connected");
    }
}

static void
disconnect_impl (gpointer data, gpointer user_data)
{
  AtmotubeData *d = (AtmotubeData *) data;
  int *ret = (int *) user_data;

  if (d->connected)
    {
      PRINT_DEBUG ("Disconnecting %s\n", d->device.device_address);

      if (gattlib_disconnect (d->connection) == 0)
	{
	  PRINT_DEBUG ("Disconnected from %s\n", d->device.device_address);
	  d->connected = false;
	}
      else
	{
	  PRINT_DEBUG ("%s\n", "gattlib_disconnect failed");
	  *ret += 1;
	  d->connected = false;
	}
    }
}

int
atmotube_connect ()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, connect_impl, &ret);

  if (ret != 0)
    {
      return ATMOTUBE_RET_ERROR;
    }
  else
    {
      return ATMOTUBE_RET_OK;
    }
}

int
atmotube_disconnect ()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, disconnect_impl, &ret);

  if (ret != 0)
    {
      return ATMOTUBE_RET_ERROR;
    }
  else
    {
      return ATMOTUBE_RET_OK;
    }
}

static void
modify_intervals (AtmotubeData * d, bool add_interval)
{
  if (add_interval)
    {
      PRINT_DEBUG ("%s\n", "Adding intervals");
    }
  else
    {
      PRINT_DEBUG ("%s\n", "Removing intervals");
    }

  uint8_t character_id;
  uint16_t interval = INTERVAL_SEC_TO_MS (d->device.device_resolution);
  for (character_id = VOC; character_id < CHARACTER_MAX; character_id++)
    {
      const char *label = intervalnames[character_id];
      const char *fmt = fmts[character_id];
      if (strlen (fmt) > 0)
	{

	  if (add_interval)
	    {
	      PRINT_DEBUG ("Adding interval: %d:%s:%s\n", d->device.device_id,
			   label, fmt);
	      interval_add (d->device.device_id, label, fmt);
	      interval_start (d->device.device_id, label, fmt, interval);

	      switch (character_id)
		{
		case VOC:
		  interval_add_float_callback (d->device.device_id, label,
					       fmt, output_voc, d);
		  break;
		case HUMIDITY:
		  interval_add_ulong_callback (d->device.device_id, label,
					       fmt, output_humidity, d);
		  break;
		case TEMPERATURE:
		  interval_add_ulong_callback (d->device.device_id, label,
					       fmt, output_temperature, d);
		  break;
		case STATUS:
		  break;
		}
	    }
	  else
	    {
	      PRINT_DEBUG ("Removing interval: %s:%s\n", label, fmt);
	      interval_remove_callbacks (d->device.device_id, label, fmt);
	      interval_stop (d->device.device_id, label, fmt);
	      interval_remove (d->device.device_id, label, fmt);
	    }
	}
    }
}

static void
register_impl (gpointer data, gpointer user_data)
{
  AtmotubeData *d = (AtmotubeData *) data;
  int *ud = (int *) user_data;
  int ret = 0;

  if (!d->connected)
    {
      *ud += 1;
      PRINT_DEBUG ("Unable to register, not connected to %s\n",
		   d->device.device_address);
      d->connected = false;
      return;
    }

  PRINT_DEBUG ("%s\n", "Register notification");
  gattlib_register_notification (d->connection, atmotube_handle_notification,
				 d);
  PRINT_DEBUG ("%s\n", "Register notification done");

  /* Add intervals. */
  modify_intervals (d, true);

  ret = 0;

  uint8_t character_id;
  for (character_id = VOC; character_id < CHARACTER_MAX; character_id++)
    {
      PRINT_DEBUG ("Notify on: %u\n", character_id);
      ret += atmotube_notify_on_characteristic (d->connection, character_id);
    }

  if (ret != 0)
    {
      PRINT_DEBUG ("atmotube_notify_on_characteristic failed for %s\n",
		   d->device.device_address);
      modify_intervals (d, false);
      gattlib_disconnect (d->connection);
      d->connected = false;
      *ud += 1;
      return;
    }
}

static void
unregister_impl (gpointer data, gpointer user_data)
{
  AtmotubeData *d = (AtmotubeData *) data;
  UNUSED (user_data);
  int ret = 0;

  modify_intervals (d, false);

  if (d->connected)
    {
      uint8_t character_id;
      for (character_id = VOC; character_id < CHARACTER_MAX; character_id++)
	{
	  PRINT_DEBUG ("Stop notify on: %u\n", character_id);
	  ret += atmotube_stop_notification (d->connection, character_id);
	}
    }
}

int
atmotube_register ()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, register_impl, &ret);

  if (ret != 0)
    {
      return ATMOTUBE_RET_ERROR;
    }
  else
    {
      return ATMOTUBE_RET_OK;
    }
}

int
atmotube_unregister ()
{
  int ret = 0;
  g_slist_foreach (glData.connectableDevices, unregister_impl, &ret);

  if (ret != 0)
    {
      return ATMOTUBE_RET_ERROR;
    }
  else
    {
      return ATMOTUBE_RET_OK;
    }
}

uuid_t *
atmotube_getuuid (enum CHARACTER_ID id)
{
  return &UUIDS[id];
}

static void
freeFoundDevice (gpointer data, gpointer user_data)
{
  char *deviceName = (char *) data;
  UNUSED (user_data);
  free (deviceName);
}

static void
freeFoundDevices (void)
{
  if (glData.foundDevices != NULL)
    {
      g_slist_foreach (glData.foundDevices, freeFoundDevice, NULL);
      g_slist_free (glData.foundDevices);
      glData.foundDevices = NULL;
    }

  if (glData.found_devices_output != NULL)
    {
      free (glData.found_devices_output);
      glData.found_devices_output = NULL;
    }

  if (glData.search_name != NULL)
    {
      free (glData.search_name);
      glData.search_name = NULL;
    }
}

void
atmotube_end ()
{
  /* Disconnect, remove any outputs and plugins. */
  atmotube_disconnect ();
  atmotube_unregister ();
  atmotube_destroy_outputs ();
  atmotube_plugin_unload_all ();
  freeFoundDevices ();
}
