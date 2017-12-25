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

#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <atmotube.h>
#include <atmotube-config.h>
#include <atmotube-plugin.h>
#include <atmotube-private.h>
#include <atmotube-interval.h>
#include <atmotube-handler.h>
#include <db.h>
#include <unistd.h>

START_TEST (test_db_plugin)
{
    PRINT_DEBUG("%s\n", "test");

    AtmotubeOutput* o = NULL;

    int ret = plugin_start(o);
}
END_TEST

Suite* atmreader_db_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("atmreader db");

    /* Core test case */
    tc_core = tcase_create("Core");
    /* Set timeout, as test_multi_interval can take a while. */
    tcase_set_timeout(tc_core, 30);
    tcase_add_test(tc_core, test_db_plugin);
    suite_add_tcase(s, tc_core);
    return s;
}

Suite *suites [] = {
    atmreader_db_suite
};
bool run_init = false;
