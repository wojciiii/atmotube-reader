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

#ifndef DB_H
#define DB_H

#include "atmotube-plugin-if.h"

int db_plugin_setup_database(const char *file_name,
			     const char *device_name,
			     const char *device_address);

int db_plugin_create_tables(void);

int db_plugin_create_statements(void);

int db_plugin_find_device(const char *name, const char *address, int *id);

int db_plugin_insert_device(const char *name, const char* address);

#endif /* DB_H */

