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

#ifndef ATMOTUBE_HANDLER_H
#define ATMOTUBE_HANDLER_H

#include <stdlib.h>
#include <unistd.h>

void atmotube_handle_notification(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);

int atmotube_notify_on_characteristic(gatt_connection_t* connection, enum CHARACTER_ID id);
int atmotube_stop_notification(gatt_connection_t* connection, enum CHARACTER_ID id);

#endif /* ATMOTUBE_HANDLER_H */
