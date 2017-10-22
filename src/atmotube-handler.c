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

static void atmotube_handle_voc(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 2) {
	PRINT_DEBUG("handle_voc: no data\n");
	return;
    }

    uint16_t voc_input = *(data+1) | ((uint16_t)*(data)) << 8;
    double voc = voc_input / 100.0f;
    PRINT_DEBUG("handle_voc: 0x%x, %f\n", voc_input, voc);

    interval_log(intervalnames[VOC], fmts[VOC], voc);
}

static void atmotube_handle_humidity(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 1) {
	PRINT_DEBUG("handle_humidity: no data\n");
	return;
    }

    uint16_t humidity = data[0] & 0xFF;
    PRINT_DEBUG("handle_humidity: 0x%x, %d%%\n", data[0], humidity);
    interval_log(intervalnames[HUMIDITY], fmts[HUMIDITY], humidity);
}

static void atmotube_handle_temperature(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 1) {
	PRINT_DEBUG("handle_temperature: no data\n");
	return;
    }

    uint16_t temperature = data[0] & 0xFF;
    PRINT_DEBUG("handle_temperature: 0x%x, %d C\n", data[0], temperature);
    interval_log(intervalnames[TEMPERATURE], fmts[TEMPERATURE], temperature);
}

static void atmotube_handle_status(const uint8_t* data, size_t data_length)
{
    if ((data_length) < 1) {
	PRINT_DEBUG("handle_status: no data\n");
	return;
    }

    /* ASDFGHJK                                          */
    /* A  (1b)  - 0 -> 3 sec, 1 -> 30 sec.               */
    /* S  (1b)  - 0 -> calibrating, 1 -> ready.          */
    /* DF (2b)  - reserved.                              */
    /* G  (1b)  - 0 -> not charging, 1 -> charging.      */
    /* HJK (3b) - battery charging level, 25% incremets. */

    const uint8_t first_bit_mask = 0x1;
    const uint8_t mode_offset  = 8;
    const uint8_t calib_offset = 7;
    const uint8_t battery_mask = 0x7;
    const uint8_t charging_offset = 3;

    bool mode = (data[0] >> mode_offset) & first_bit_mask;
    bool calibrating = (data[0] >> calib_offset) & first_bit_mask;
    uint8_t battery_percent = (data[0] & battery_mask) * 25;
    bool charging = (data[0] >> charging_offset) & first_bit_mask;

    printf("Status:\n");

    if (mode) {
	printf("\tmode: slow\n");
    } else {
	printf("\tmode: fast\n");
    }

    if (calibrating) {
	printf("\tdev: calibrating\n");
    } else {
	printf("\tdev: ready\n");
    }

    if (charging) {
	printf("\tpower: charging\n");
    } else {
	printf("\tpower: not charging\n");
    }

    printf("\tbattery: %u%%\n", battery_percent);
}

void atmotube_handle_notification(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
  int i;
  enum CHARACTER_ID id = CHARACTER_MAX;
  AtmotubeData* d = (AtmotubeData*)user_data;

  PRINT_DEBUG("Notification Handler for %s:\n", d->device.device_address);

  for (i = VOC; i < CHARACTER_MAX; i++)
  {
    if (gattlib_uuid_cmp(uuid, &UUIDS[i]) == 0)
    {
      PRINT_DEBUG("Found id %d\n", i);
      id = (enum CHARACTER_ID)i;
      break;
    }
  }

  switch (id)
  {
    case VOC:
      PRINT_DEBUG("VOC\n");
      atmotube_handle_voc(data, data_length);
      break;
    case HUMIDITY:
      PRINT_DEBUG("HUMIDITY\n");
      atmotube_handle_humidity(data, data_length);
      break;
    case TEMPERATURE:
      PRINT_DEBUG("TEMPERATURE\n");
      atmotube_handle_temperature(data, data_length);
      break;
    case STATUS:
      PRINT_DEBUG("STATUS\n");
      atmotube_handle_status(data, data_length);
      break;
    default:
      PRINT_DEBUG("UNKN %d\n", id);
      break;
  }

  for (i = 0; i < data_length; i++)
  {
    PRINT_DEBUG("%02x ", data[i]);
  }
  PRINT_DEBUG("\n");
}
