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

#ifndef ATMOTUBE_H
#define ATMOTUBE_H

#include "gattlib.h"

/*
  Taken from: https://atmotube.com/api.html:
  
  db450001-8e9a-4818-add7-6ed94a328ab2 Atmotube service UUID
  db450002-8e9a-4818-add7-6ed94a328ab2 VOC characteristic
  db450003-8e9a-4818-add7-6ed94a328ab2 Humidity characteristic
  db450004-8e9a-4818-add7-6ed94a328ab2 Temperature characteristic
  db450005-8e9a-4818-add7-6ed94a328ab2 Status characteristic
*/
enum CHARACTER_ID
{
  VOC = 0,
  HUMIDITY,
  TEMPERATURE,
  STATUS,
  CHARACTER_MAX
};

void atmotube_start();
void atmotube_end();

void handle_notification(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);

int notify_on_characteristic(gatt_connection_t* connection, enum CHARACTER_ID id);
int stop_notification(gatt_connection_t* connection, enum CHARACTER_ID id);

void handle_voc(const uint8_t* data, size_t data_length);
void handle_humidity(const uint8_t* data, size_t data_length);
void handle_temperature(const uint8_t* data, size_t data_length);
void handle_status(const uint8_t* data, size_t data_length);

#endif /* ATMOTUBE_H */
