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
#include <unistd.h>
#include <atmotube.h>

#include "atmotube-test-common.h"

int
exec_suite (Suite * s, bool atm_init)
{
  int number_failed = 0;
  SRunner *sr;

  if (atm_init)
    {
      atmotube_start ();
    }

  sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  if (atm_init)
    {
      atmotube_end ();
    }

  return number_failed;
}

int
main (void)
{
  int number_failed = 0;

  Suite *suites[num_suites];
  Suite *s = NULL;

  for (uint16_t i = 0; i < num_suites; i++)
    {
      SuiteCallback cb = suite_callbacks[i];
      suites[i] = cb ();
    }

  for (uint16_t i = 0; i < num_suites; i++)
    {
      s = suites[i];
      number_failed += exec_suite (s, run_init);
    }

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
