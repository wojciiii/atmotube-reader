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

#ifndef ATMOTUBE_COMMON_H
#define ATMOTUBE_COMMON_H

#define ATMOTUBE_RET_OK 0
#define ATMOTUBE_RET_ERROR 1

#ifndef DEBUG
#define DEBUG 1
#endif

#if (DEBUG)
#  define PRINT_DEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#  define PRINT_DEBUG(fmt, ...)
#endif

#endif /* ATMOTUBE_COMMON_H */