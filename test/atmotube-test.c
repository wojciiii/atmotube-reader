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
#include <unistd.h>

#include "atmotube-test-common.h"

static char* plugin_path = NULL;

static void set_plugin_path(const char* path)
{
    printf("Plugin path: %s\n", path);

    ck_assert(plugin_path == NULL);
    
    plugin_path = strdup(path);
}
/*
typedef struct
{
    // ..
    char* deviceAddress;
    // ..
} AtmotubeData;
*/

void test_handle_notification(enum CHARACTER_ID id, uint8_t* data, size_t data_length)
{
    uuid_t* uuid = atmotube_getuuid(id);
    AtmotubeData d;

    d.device.device_address = "00:00:00:00:00";
	
    //d.deviceAddress = "00:00:00:00:00";
    void* user_data = &d;

    atmotube_handle_notification(uuid, data, data_length, user_data);
}

START_TEST (test_handle_VOC_notification)
{
    //uint8_t data[] = {0x01, 0xb9};
    uint8_t data[] = {0x02, 0x94};
    size_t data_length = sizeof(data);
    test_handle_notification(VOC, &data[0], data_length);
}
END_TEST

START_TEST (test_handle_HUMIDITY_notification)
{
    uint8_t data[] = {0x1e};
    size_t data_length = sizeof(data);
    test_handle_notification(HUMIDITY, &data[0], data_length);
}
END_TEST

START_TEST (test_handle_TEMPERATURE_notification)
{
    uint8_t data[] = {0x1c};
    size_t data_length = sizeof(data);
    test_handle_notification(TEMPERATURE, &data[0], data_length);
}
END_TEST

START_TEST (test_handle_STATUS_notification)
{
    uint8_t data[] = {0x41};
    size_t data_length = sizeof(data);
    test_handle_notification(STATUS, &data[0], data_length);

    /* 100 % charged */
    uint8_t data2[] = {0x5C};
    data_length = sizeof(data2);
    test_handle_notification(STATUS, &data2[0], data_length);
}
END_TEST

static Atmotube_Device* deviceStore = NULL;

static void* dummy1(int num_devices)
{
    deviceStore = (Atmotube_Device*)malloc(num_devices * sizeof(Atmotube_Device));
    return deviceStore;
}

static int dummy2(void* memory)
{
    Atmotube_Device* d = (Atmotube_Device*)memory;
    printf("Callback device: %s\n", d->device_name);
    
    return 0;
}

START_TEST (test_load_config)
{
    const char* fullName = "../test/config.txt";
    atmotube_config_start(fullName);

    int ret = atmotube_config_load(set_plugin_path, dummy1, dummy2, sizeof(Atmotube_Device), 0);
    if (ret == 0)
    {
        atmotube_config_end();
    }
    free(deviceStore);
    deviceStore = NULL;

    atmotube_config_end();
    free(plugin_path);
    plugin_path = NULL;
}
END_TEST

typedef struct
{
    int somedata;
    uint32_t someotherData;
    Atmotube_Device device;
    uint32_t evenmoredata;    
} StructWithOffset;

static StructWithOffset* deviceStore2 = NULL;

static void* dummy4(int num_devices)
{
    deviceStore2 = (StructWithOffset*)malloc(num_devices * sizeof(StructWithOffset));
    memset(deviceStore2, 0, num_devices * sizeof(StructWithOffset));
    return deviceStore2;
}

static int dummy5(void* memory)
{
    Atmotube_Device* d = (Atmotube_Device*)memory;
    printf("Callback device: %s\n", d->device_name);
    return 0;
}

START_TEST (test_load_config_offset)
{
    char* fullName = "../test/config.txt";
    atmotube_config_start(fullName);

    size_t offset = offsetof(StructWithOffset, device);
    printf("Using offset: %d\n", offset);
    int ret = atmotube_config_load(set_plugin_path, dummy4, dummy5, sizeof(StructWithOffset), offset);
    if (ret == 0)
    {
        atmotube_config_end();
    }
    free(deviceStore2);
    deviceStore2 = NULL;

    atmotube_config_end();
    free(plugin_path);
    plugin_path = NULL;
}
END_TEST

static void* p1 = (void*)0x1;
static void* p2 = (void*)0x2;

void callback_ulong(unsigned long ts, unsigned long value, void* data_ptr)
{
    printf("Time: %lu, value=%lu\n", ts, value);
    ck_assert(data_ptr == p1);
}

void callback_float(unsigned long ts, float value, void* data_ptr)
{
    printf("Time: %lu, value=%f\n", ts, value);
    ck_assert(data_ptr == p2);
}

START_TEST (test_interval)
{
    int i;
    int device_id = 0;
    const char* TEST1 = "test1";
    const char* TEST2 = "test2";
    const char* TEST3 = "test3";
    
    // LABEL, TYPE
    interval_add(device_id, TEST1, INTERVAL_ULONG);
    interval_add(device_id, TEST2, INTERVAL_FLOAT);
    interval_add(device_id, TEST3, INTERVAL_ULONG);

    
    interval_add_ulong_callback(device_id, TEST1, INTERVAL_ULONG, callback_ulong, p1);
    interval_add_float_callback(device_id, TEST2, INTERVAL_FLOAT, callback_float, p2);
    interval_add_ulong_callback(device_id, TEST3, INTERVAL_ULONG, callback_ulong, p1);
 
    //interval_dump();

    unsigned int t1 = 1000;
    double f1 = 0.52f;
    unsigned int t3 = 2000;
    
    interval_start(device_id, TEST1, INTERVAL_ULONG, 1000);
    interval_start(device_id, TEST2, INTERVAL_FLOAT, 550);
    interval_start(device_id, TEST3, INTERVAL_ULONG, 500);

    for (i = 0; i < 21; i++)
    {
        interval_log(device_id, TEST1, INTERVAL_ULONG, t1);
	interval_log(device_id, TEST2, INTERVAL_FLOAT, f1);
	interval_log(device_id, TEST3, INTERVAL_ULONG, t3);
        usleep(100*1000);
    }

    interval_stop(device_id, TEST1, INTERVAL_ULONG);
    interval_stop(device_id, TEST2, INTERVAL_FLOAT);
    interval_stop(device_id, TEST3, INTERVAL_ULONG);

    interval_remove(device_id, TEST1, INTERVAL_ULONG);
    interval_remove(device_id, TEST2, INTERVAL_FLOAT);
    interval_remove(device_id, TEST3, INTERVAL_ULONG);
}
END_TEST

static void* device_ptr[2] = { (void*)0x1, (void*)0x2 };

static int called_dev0 = 0;
static int called_dev1 = 0;

static void multi_callback_ulong_dev0(unsigned long ts, unsigned long value, void* data_ptr)
{
    printf("Time(0): %lu, value=%lu\n", ts, value);
    ck_assert(data_ptr == device_ptr[0]);
    called_dev0++;
}

static void multi_callback_float_dev0(unsigned long ts, float value, void* data_ptr)
{
    printf("Time(0): %lu, value=%f\n", ts, value);
    ck_assert(data_ptr == device_ptr[0]);
    called_dev0++;
}

static void multi_callback_ulong_dev1(unsigned long ts, unsigned long value, void* data_ptr)
{
    printf("Time(1): %lu, value=%lu\n", ts, value);
    ck_assert(data_ptr == device_ptr[1]);
    called_dev1++;
}

static void multi_callback_float_dev1(unsigned long ts, float value, void* data_ptr)
{
    printf("Time(1): %lu, value=%f\n", ts, value);
    ck_assert(data_ptr == device_ptr[1]);
    called_dev1++;
}

START_TEST (test_multi_interval)
{
    int i;
    int device_id;
    int max_dev_id = 2;
    const char* TEST1 = "test1";
    const char* TEST2 = "test2";
    const char* TEST3 = "test3";

    device_id = 0;

    // LABEL, TYPE
    interval_add(device_id, TEST1, INTERVAL_ULONG);
    interval_add(device_id, TEST2, INTERVAL_FLOAT);
    interval_add(device_id, TEST3, INTERVAL_ULONG);
    
    interval_add_ulong_callback(device_id, TEST1, INTERVAL_ULONG, multi_callback_ulong_dev0, device_ptr[0]);
    interval_add_float_callback(device_id, TEST2, INTERVAL_FLOAT, multi_callback_float_dev0, device_ptr[0]);
    interval_add_ulong_callback(device_id, TEST3, INTERVAL_ULONG, multi_callback_ulong_dev0, device_ptr[0]);

    device_id = 1;

    // LABEL, TYPE
    interval_add(device_id, TEST1, INTERVAL_ULONG);
    interval_add(device_id, TEST2, INTERVAL_FLOAT);
    interval_add(device_id, TEST3, INTERVAL_ULONG);

    interval_add_ulong_callback(device_id, TEST1, INTERVAL_ULONG, multi_callback_ulong_dev1, device_ptr[1]);
    interval_add_float_callback(device_id, TEST2, INTERVAL_FLOAT, multi_callback_float_dev1, device_ptr[1]);
    interval_add_ulong_callback(device_id, TEST3, INTERVAL_ULONG, multi_callback_ulong_dev1, device_ptr[1]);

    interval_dump();

    unsigned int t1 = 1000;
    double f1 = 0.52f;
    unsigned int t3 = 2000;

    for (device_id = 0; device_id < max_dev_id; device_id++) {    
	interval_start(device_id, TEST1, INTERVAL_ULONG, 1000);
	interval_start(device_id, TEST2, INTERVAL_FLOAT, 550);
	interval_start(device_id, TEST3, INTERVAL_ULONG, 500);
    }

    for (i = 0; i < 100; i++) {
	for (device_id = 0; device_id < max_dev_id; device_id++) {
	    interval_log(device_id, TEST1, INTERVAL_ULONG, t1);
	    interval_log(device_id, TEST2, INTERVAL_FLOAT, f1);
	    interval_log(device_id, TEST3, INTERVAL_ULONG, t3);
	}
	usleep(10*1000);
    }

    for (device_id = 0; device_id < max_dev_id; device_id++) {    

	interval_stop(device_id, TEST1, INTERVAL_ULONG);
	interval_stop(device_id, TEST2, INTERVAL_FLOAT);
	interval_stop(device_id, TEST3, INTERVAL_ULONG);

	interval_remove(device_id, TEST1, INTERVAL_ULONG);
	interval_remove(device_id, TEST2, INTERVAL_FLOAT);
	interval_remove(device_id, TEST3, INTERVAL_ULONG);
    }

    ck_assert(called_dev0 > 0);
    ck_assert(called_dev1 > 0);
}
END_TEST

static StructWithOffset* deviceStore3 = NULL;

static void* dummy7(int num_devices)
{
    deviceStore3 = (StructWithOffset*)malloc(num_devices * sizeof(StructWithOffset));
    memset(deviceStore3, 0, num_devices * sizeof(StructWithOffset));
    return deviceStore3;
}

static int dummy8(void* memory)
{
    Atmotube_Device* d = (Atmotube_Device*)memory;
    printf("Callback device: %s\n", d->device_name);
    return 0;
}

START_TEST (test_plugin)
{
    const char* fullName = "../test/config.txt";
    atmotube_config_start(fullName);

    size_t offset = offsetof(StructWithOffset, device);
    printf("Using offset: %d\n", offset);
    int ret = atmotube_config_load(set_plugin_path, dummy7, dummy8, sizeof(StructWithOffset), offset);
    if (ret == 0)
    {
        atmotube_config_end();
    }

    ck_assert(ret == ATMOTUBE_RET_OK);
    ck_assert(plugin_path != NULL);
    
    ret = atmotube_plugin_find(plugin_path);
    ck_assert(ret == ATMOTUBE_RET_OK);

    AtmotubePlugin* o = atmotube_plugin_get(OUTPUT_FILE);
    ck_assert(o != NULL);
    o = atmotube_plugin_get(OUTPUT_DB);
    ck_assert(o != NULL);
    o = atmotube_plugin_get(OUTPUT_CUSTOM);
    ck_assert(o != NULL);

    atmotube_plugin_unload_all();

    free(deviceStore3);
    deviceStore3 = NULL;

    atmotube_config_end();
    free(plugin_path);
    plugin_path = NULL;
}
END_TEST

START_TEST (test_output)
{
    const char* fullName = "../test/config.txt";
    int ret;
    
    atmotube_start();

    ret = atmotube_add_devices_from_config(fullName);
    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to add devices from config.\n");
        atmotube_end();
	return;
    }

    ret = atmotube_create_outputs();
    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to create output(s).\n");
        atmotube_end();
    }

}
END_TEST

START_TEST (test_output_db)
{
    const char* fullName = "../test/config-db.txt";
    int ret;

    atmotube_start();

    ret = atmotube_add_devices_from_config(fullName);

    ck_assert(ret == ATMOTUBE_RET_OK);
	
    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to add devices from config.\n");
        atmotube_end();
	return;
    }

    ret = atmotube_create_outputs();

    ck_assert(ret == ATMOTUBE_RET_OK);

    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to create output(s).\n");
        atmotube_end();
	return;
    }

    /* Test that writting to an output plugin works and it generated
     * the expected output.
     */

    extern AtmotubeGlData glData;

    printf("Num %d\n", glData.deviceConfigurationSize);
    
    AtmotubeData* target = NULL;
    int i;
    for (i = 0; i < glData.deviceConfigurationSize; i++) {
	printf("Data %d\n", i);
	
	AtmotubeData* d = glData.deviceConfiguration + i;
	AtmotubePlugin* plugin = d->plugin;
	if (plugin != NULL) {
	    if (strcmp(plugin->type, OUTPUT_DB) == 0) {
		printf("Found it\n");
		target = d;
		break;
	    }
	}
    }
    
    ck_assert(target != NULL);
    
    unsigned long ts = 0;
    unsigned long value = 100UL;
    void* data_ptr = target;

    for (i = 0; i < 5; i++) {
	output_temperature(ts+i, value+i, data_ptr);
	output_humidity(ts+i, value+i, data_ptr);
	output_voc(ts+i, value+i+0.1f, data_ptr);
    }

    atmotube_end();
}
END_TEST
     
START_TEST (test_output_file)
{
    const char* fullName = "../test/config.txt";
    int ret;

    atmotube_start();

    ret = atmotube_add_devices_from_config(fullName);

    ck_assert(ret == ATMOTUBE_RET_OK);
	
    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to add devices from config.\n");
        atmotube_end();
	return;
    }

    ret = atmotube_create_outputs();

    ck_assert(ret == ATMOTUBE_RET_OK);

    if (ret != ATMOTUBE_RET_OK) {
	printf("Unable to create output(s).\n");
        atmotube_end();
	return;
    }

    /* Test that writting to an output plugin works and it generated
     * the expected output.
     */

    extern AtmotubeGlData glData;

    printf("Num %d\n", glData.deviceConfigurationSize);
    
    AtmotubeData* target = NULL;
    int i;
    for (i = 0; i < glData.deviceConfigurationSize; i++) {
	printf("Data %d\n", i);
	
	AtmotubeData* d = glData.deviceConfiguration + i;
	AtmotubePlugin* plugin = d->plugin;
	if (plugin != NULL) {
	    if (strcmp(plugin->type, OUTPUT_FILE) == 0) {
		printf("Found it\n");
		target = d;
		break;
	    }
	}
    }
    
    ck_assert(target != NULL);
    
    unsigned long ts = 0;
    unsigned long value = 100UL;
    void* data_ptr = target;

    for (i = 0; i < 5; i++) {
	output_temperature(ts+i, value+i, data_ptr);
	output_humidity(ts+i, value+i, data_ptr);
	output_voc(ts+i, value+i+0.1f, data_ptr);
    }

    atmotube_end();
}
END_TEST

Suite* atmreader_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("atmreader");

    /* Core test case */
    tc_core = tcase_create("Core");
    /* Set timeout, as test_multi_interval can take a while. */
    tcase_set_timeout(tc_core, 30);
    /* Inidividual testcases. */
    tcase_add_test(tc_core, test_interval);
    /*
    tcase_add_test(tc_core, test_multi_interval);
    tcase_add_test(tc_core, test_handle_VOC_notification);
    tcase_add_test(tc_core, test_handle_TEMPERATURE_notification);
    tcase_add_test(tc_core, test_handle_HUMIDITY_notification);
    tcase_add_test(tc_core, test_handle_STATUS_notification);
    tcase_add_test(tc_core, test_load_config);
    tcase_add_test(tc_core, test_load_config_offset);
    tcase_add_test(tc_core, test_plugin);
    tcase_add_test(tc_core, test_output);
    */
    //tcase_add_test(tc_core, test_output_file);
    tcase_add_test(tc_core, test_output_db);

    //tcase_add_test(tc_core, test_db_plugin);
    suite_add_tcase(s, tc_core);
    return s;
}

SuiteCallback suite_callbacks[] = {
    atmreader_suite
};

int num_suites = sizeof(suite_callbacks) / sizeof(SuiteCallback);

bool run_init = true;
