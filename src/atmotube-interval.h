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

#ifndef INTERVAL_H
#define INTERVAL_H

/* Supported intervals. */
#define INTERVAL_ULONG "%lu"
#define INTERVAL_FLOAT "%f"

#define INTERVAL_SEC_TO_MS(x) (1000*x)

/* Define an interval described by a label and a format. */
int interval_add(int device_id, const char *label, const char *fmt);

typedef void (*ulong_callback)(unsigned long, unsigned long, void* data_ptr);
typedef void (*float_callback)(unsigned long, float, void* data_ptr);

int interval_add_ulong_callback(int device_id,
				const char *label,
				const char *fmt,
				ulong_callback callback,
				void* data_ptr);

int interval_add_float_callback(int device_id,
				const char *label,
				const char *fmt,
				float_callback callback,
				void* data_ptr);

int interval_remove_callbacks(int device_id,
			      const char *label,
			      const char *fmt);

/* Remove a previously added interval. */
int interval_remove(int device_id, const char *label, const char *fmt);

/* Start a previously defined interval. */
int interval_start(int device_id, const char *label, const char *fmt, unsigned long interval_ms);
/* Start a previously started interval. */
int interval_stop(int device_id, const char *label, const char *fmt);

/* Log some data using a previously defined interval. */
void interval_log(int device_id, const char *label, const char *fmt, ...);

/* Print the defined intervals. */
void interval_dump(void);

#endif /* INTERVAL_H */
