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

#ifndef ATMOTUBE_OUTPUT_H
#define ATMOTUBE_OUTPUT_H

/* Create output plugins described by config. */
int atmotube_create_outputs();

void output_temperature(unsigned long ts, unsigned long value, void* data_ptr);
void output_humidity(unsigned long ts, unsigned long value, void* data_ptr);
void output_voc(unsigned long ts, float value, void* data_ptr);

#endif /* ATMOTUBE_OUTPUT_H */
