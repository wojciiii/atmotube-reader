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
#include <interval.h>
#include <unistd.h>

typedef struct
{
    // ..
    char* deviceAddress;
    // ..
} AtmotubeData;

void test_handle_notification(enum CHARACTER_ID id, uint8_t* data, size_t data_length)
{
    uuid_t* uuid = atmotube_getuuid(id);
    AtmotubeData d;
    d.deviceAddress = "00:00:00:00:00";
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
    char* fullName = "../test/config.txt";
    atmotube_config_start(fullName);

    int ret = atmotube_config_load(dummy1, dummy2, sizeof(Atmotube_Device), 0);
    if (ret == 0)
    {
        atmotube_config_end();
    }
    free(deviceStore);
    deviceStore = NULL;
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

static void* dummy3(int num_devices)
{
    deviceStore2 = (StructWithOffset*)malloc(num_devices * sizeof(StructWithOffset));
    memset(deviceStore2, 0, num_devices * sizeof(StructWithOffset));
    return deviceStore2;
}

static int dummy4(void* memory)
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
    int ret = atmotube_config_load(dummy3, dummy4, sizeof(StructWithOffset), offset);
    if (ret == 0)
    {
        atmotube_config_end();
    }
    free(deviceStore2);
    deviceStore2 = NULL;
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
    const char* TEST1 = "test1";
    const char* TEST2 = "test2";
    const char* TEST3 = "test3";
    
    // LABEL, TYPE
    interval_add(TEST1, INTERVAL_ULONG);
    interval_add(TEST2, INTERVAL_FLOAT);
    interval_add(TEST3, INTERVAL_ULONG);

    
    interval_add_ulong_callback(TEST1, INTERVAL_ULONG, callback_ulong, p1);
    interval_add_float_callback(TEST2, INTERVAL_FLOAT, callback_float, p2);
    interval_add_ulong_callback(TEST3, INTERVAL_ULONG, callback_ulong, p1);
 
    //interval_dump();

    unsigned int t1 = 1000;
    double f1 = 0.52f;
    unsigned int t3 = 2000;
    
    interval_start(TEST1, INTERVAL_ULONG, 1000);
    interval_start(TEST2, INTERVAL_FLOAT, 550);
    interval_start(TEST3, INTERVAL_ULONG, 500);

    for (i = 0; i < 21; i++)
    {
        interval_log(TEST1, INTERVAL_ULONG, t1);
	interval_log(TEST2, INTERVAL_FLOAT, f1);
	interval_log(TEST3, INTERVAL_ULONG, t3);
        usleep(100*1000);
    }

    interval_stop(TEST1, INTERVAL_ULONG);
    interval_stop(TEST2, INTERVAL_FLOAT);
    interval_stop(TEST3, INTERVAL_ULONG);

    interval_remove(TEST1, INTERVAL_ULONG);
    interval_remove(TEST2, INTERVAL_FLOAT);
    interval_remove(TEST3, INTERVAL_ULONG);
}
END_TEST

Suite* atmreader_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("atmreader");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_handle_VOC_notification);
    tcase_add_test(tc_core, test_handle_TEMPERATURE_notification);
    tcase_add_test(tc_core, test_handle_HUMIDITY_notification);
    tcase_add_test(tc_core, test_handle_STATUS_notification);

    tcase_add_test(tc_core, test_load_config);
    tcase_add_test(tc_core, test_load_config_offset);

    tcase_add_test(tc_core, test_interval);

    //tcase_add_test(tc_core, test_name);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    atmotube_start();

    s = atmreader_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    atmotube_end();

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

