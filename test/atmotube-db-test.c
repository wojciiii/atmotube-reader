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

#include "atmotube-test-common.h"

char filenamebuffer[1024]; 

static AtmotubeOutput o;

static void setup_output(int counter)
{
    snprintf(&filenamebuffer[0], sizeof(filenamebuffer), "test_db_plugin-%d.db", counter);
    PRINT_DEBUG("Using counter %d, filename: %s\n", counter, &filenamebuffer[0]);
    o.filename = &filenamebuffer[0];
    o.device_name = "my device";
    o.device_address = "00:00:00:00:00:00";
}

typedef enum {
    TO_CREATE_TABLES = 0,
    TO_CREATE_STATEMENTS,
    TO_FIND_DEVICE,
    TO_INSERT_DEVICE,
    TO_DEVICE_FOUND,
    TO_TEST_DB_PLUGIN,
    TO_INSERT_VALUES
} test_output;
    
START_TEST (test_create_tables)
{
    setup_output(TO_CREATE_TABLES);
    int ret = db_plugin_setup_database(o.filename,
				       o.device_name,
				       o.device_address);
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_tables();
    ck_assert(ret == ATMOTUBE_RET_OK);
}
END_TEST

START_TEST (test_create_statements)
{
    setup_output(TO_CREATE_STATEMENTS);
    int ret = db_plugin_setup_database(o.filename,
				       o.device_name,
				       o.device_address);
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_tables();
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_statements();
    ck_assert(ret == ATMOTUBE_RET_OK);

    db_plugin_destroy_statements();
}
END_TEST

START_TEST(test_find_device_not_found)
{
    setup_output(TO_FIND_DEVICE);
    int ret = db_plugin_setup_database(o.filename,
				       o.device_name,
				       o.device_address);
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_tables();
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_statements();
    ck_assert(ret == ATMOTUBE_RET_OK);

    int found_id = -1;
    ret = db_plugin_find_device("test", "00", &found_id);
    ck_assert(ret != ATMOTUBE_RET_OK);

    db_plugin_destroy_statements();
}
END_TEST

START_TEST(test_insert_device)
{
    setup_output(TO_INSERT_DEVICE);

    int ret = db_plugin_setup_database(o.filename,
				       o.device_name,
				       o.device_address);
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_tables();
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_statements();
    ck_assert(ret == ATMOTUBE_RET_OK);

    const char *testname = "testname";
    const char *devaddress = "00:00:00:00:00:00";
    ret = db_plugin_insert_device(testname, devaddress);
    ck_assert(ret == ATMOTUBE_RET_OK);

    int found_id = -1;
    ret = db_plugin_find_device(testname, devaddress, &found_id);
    ck_assert(ret == ATMOTUBE_RET_OK);

    ck_assert(found_id != -1);

    db_plugin_destroy_statements();
}
END_TEST

START_TEST(test_find_device_found)
{
    setup_output(TO_DEVICE_FOUND);

    int ret = db_plugin_setup_database(o.filename,
				       o.device_name,
				       o.device_address);
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_tables();
    ck_assert(ret == ATMOTUBE_RET_OK);

    ret = db_plugin_create_statements();
    ck_assert(ret == ATMOTUBE_RET_OK);

    const char *testname0 = "testname0";
    const char *devaddress0 = "00:00:00:00:00:00";
    ret = db_plugin_insert_device(testname0, devaddress0);
    ck_assert(ret == ATMOTUBE_RET_OK);

    const char *testname1 = "testname1";
    const char *devaddress1 = "00:00:00:00:00:01";
    ret = db_plugin_insert_device(testname1, devaddress1);
    ck_assert(ret == ATMOTUBE_RET_OK);

    int found_id = -1;
    ret = db_plugin_find_device("test", "00", &found_id);
    ck_assert(ret != ATMOTUBE_RET_OK);

    ret = db_plugin_find_device(testname0, devaddress0, &found_id);
    ck_assert(ret == ATMOTUBE_RET_OK);

    int saved_found_id = found_id;
    ret = db_plugin_find_device(testname1, devaddress1, &found_id);
    ck_assert(ret == ATMOTUBE_RET_OK);

    PRINT_DEBUG("saved_found_id(%d) < found_id(%d)\n", saved_found_id, found_id);
    ck_assert(saved_found_id < found_id);

    db_plugin_destroy_statements();
}
END_TEST

START_TEST (test_db_plugin)
{
    setup_output(TO_TEST_DB_PLUGIN);
    int ret = plugin_start(&o);
    ck_assert(ret == ATMOTUBE_RET_OK);

    plugin_stop();
}
END_TEST

static int check_values(unsigned long time,
			unsigned long tempval,
			unsigned long humval,
			float vocval)
{
    unsigned long temp_from_db;
    unsigned long hum_from_db;
    float voc_from_db;

    get_temperature(time, &temp_from_db);
    if (temp_from_db != tempval) {
	PRINT_DEBUG("temp_from_db(%lu) != tempval(%lu)\n", temp_from_db, tempval);
	return ATMOTUBE_RET_ERROR;
    }

    get_humidity(time, &hum_from_db);
    if (hum_from_db != humval) {
	PRINT_DEBUG("hum_from_db(%lu) != humval(%lu)\n", hum_from_db, humval);
	return ATMOTUBE_RET_ERROR;
    }

    get_voc(time, &voc_from_db);
    if (voc_from_db != vocval) {
	PRINT_DEBUG("voc_from_db(%f) != vocval(%f)\n", voc_from_db, vocval);
	return ATMOTUBE_RET_ERROR;
    }

    return ATMOTUBE_RET_OK;
}

START_TEST(test_insert_values)
{
    setup_output(TO_INSERT_VALUES);
    int ret = plugin_start(&o);
    ck_assert(ret == ATMOTUBE_RET_OK);

    unsigned long tempval = 0;
    unsigned long humval = 0;
    float vocval = 0.0;

    for (unsigned long time = 0; time < 16; time ++) {
	tempval++;
	humval++;
	vocval += 0.10;

	temperature(time, tempval);
	humidity(time, humval);
	voc(time, vocval);

	ret = check_values(time, tempval, humval, vocval);
	ck_assert(ret == ATMOTUBE_RET_OK);
    }

    plugin_stop();
}
END_TEST

Suite* atmreader_db_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("atmreader db test");

    /* Core test case */
    tc_core = tcase_create("DB");
    /* Set timeout, as test_multi_interval can take a while. */
    tcase_set_timeout(tc_core, 30);
    tcase_add_test(tc_core, test_create_tables);
    tcase_add_test(tc_core, test_create_statements);
    tcase_add_test(tc_core, test_find_device_not_found);
    tcase_add_test(tc_core, test_insert_device);
    tcase_add_test(tc_core, test_find_device_found);
    tcase_add_test(tc_core, test_db_plugin);
    tcase_add_test(tc_core, test_insert_values);
    suite_add_tcase(s, tc_core);
    return s;
}

SuiteCallback suite_callbacks[] = {
    atmreader_db_suite
};
int num_suites = sizeof(suite_callbacks) / sizeof(SuiteCallback);
bool run_init = false;
