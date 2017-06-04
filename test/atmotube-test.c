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

/*
START_TEST (test_name)
{
    printf("test_name\n");
    //ck_assert(1==0);
    ck_assert(1==1);
}
END_TEST
*/

void test_handle_notification(enum CHARACTER_ID id, uint8_t* data, size_t data_length)
{
    uuid_t* uuid = atmotube_getuuid(id);
    void* user_data = NULL;

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
    uint8_t data[] = {0x26};
    size_t data_length = sizeof(data);

    test_handle_notification(HUMIDITY, &data[0], data_length);
}
END_TEST

START_TEST (test_handle_TEMPERATURE_notification)
{
    uint8_t data[] = {0x20};
    size_t data_length = sizeof(data);

    test_handle_notification(TEMPERATURE, &data[0], data_length);
}
END_TEST

START_TEST (test_handle_STATUS_notification)
{
    // TODO: test this
    uint8_t data[] = {0x41};
    size_t data_length = sizeof(data);

    test_handle_notification(STATUS, &data[0], data_length);
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

