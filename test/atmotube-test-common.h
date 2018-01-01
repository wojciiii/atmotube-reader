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

#ifndef ATMOTUBE_TEST_COMMON_H
#define ATMOTUBE_TEST_COMMON_H

#include <stdbool.h>
#include <check.h>

int exec_suite (Suite * s, bool atm_init);

/* Run atmotube init? */
extern bool run_init;
/* Number of suites to execute. */
extern int num_suites;

typedef Suite *(*SuiteCallback) (void);

/* Array of callbacks to generate test suites. */
extern SuiteCallback suite_callbacks[];

/* Array of suites. */
/* extern Suite *suites[]; */

#endif /* ATMOTUBE_TEST_COMMON_H */
