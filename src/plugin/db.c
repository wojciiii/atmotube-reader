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

#include "db.h"
#include <stdbool.h>
#include <stdio.h>
#include <atmotube.h>
#include "atmotube-config.h"
#include <sqlite3.h>

static bool started = false;
static const char *type = "db";
static sqlite3 *datbase_handle = NULL;
static const char *filename = NULL;

/* Statements used by this plugin. */

typedef enum
{
  SQLS_INSERT_DEVICE = 0,
  SQLS_SELECT_DEVICE,

  SQLS_INSERT_TEMP,
  SQLS_INSERT_HUM,
  SQLS_INSERT_VOC,

  SQLS_GET_TEMP,
  SQLS_GET_HUM,
  SQLS_GET_VOC,

  SQLS_MAX
} sql_statement;

static sqlite3_stmt *sql_statements[SQLS_MAX] = { NULL };

static int device_row_id = -1;

const char *
get_plugin_type (void)
{
  return type;
}

#define RETURN_ATM_ERROR(ret, msg) do                                   \
        {                                                               \
            if (ret != SQLITE_OK) {                                     \
                PRINT_ERROR("%s(%s)\n", msg, sqlite3_errmsg(datbase_handle)); \
                return ATMOTUBE_RET_ERROR;                              \
            }                                                           \
        } while(0)

int
db_plugin_create_tables (void)
{
  const char *statements[] = {
    "CREATE TABLE IF NOT EXISTS `voc` ( \
        `device_id` INTEGER NOT NULL,       \
        `time`  INTEGER NOT NULL,           \
        `value`         REAL NOT NULL,              \
        `description` VARCHAR(255) NOT NULL);",
    /* */
    "CREATE TABLE IF NOT EXISTS `temperature` (     \
        `device_id` INTEGER NOT NULL,                   \
        `time`  NUMERIC NOT NULL,                       \
        `value`         INTEGER NOT NULL);",
    /* */
    "CREATE TABLE IF NOT EXISTS `device` (                  \
        `id`  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,       \
        `name`  VARCHAR(255) NOT NULL,                          \
        `address` VARCHAR(255) NOT NULL);",
    /* */
    "CREATE TABLE IF NOT EXISTS `humidity` ( \
        `time`  INTEGER NOT NULL,                \
        `device_id` INTEGER NOT NULL,       \
        `value`         INTEGER NOT NULL,                \
        PRIMARY KEY(`time`) );",
    /* */
    "CREATE INDEX IF NOT EXISTS `voc_index` ON `voc` ( \
        `time` ASC,                                        \
        `value` ASC,                                       \
        `description` ASC,                                 \
        `device_id` ASC);",
    /* */
    "CREATE UNIQUE INDEX IF NOT EXISTS `temperature_index` ON `temperature` ( \
         `time` ASC,                                                    \
         `value` ASC,                                                   \
         `device_id` ASC);",
    /* */
    "CREATE UNIQUE INDEX IF NOT EXISTS `humidity_index` ON `humidity` ( \
        `time` ASC,                                                     \
        `value` ASC,                                                    \
        `device_id` ASC);",
    /* */
    "CREATE UNIQUE INDEX IF NOT EXISTS `device_index` ON `device` ( \
        `name`  ASC, \
        `address` ASC);"
  };

  uint16_t num_statements = sizeof (statements) / sizeof (char *);
  sqlite3_stmt *createStmt;

  int i = 0;
  int ret = 0;
  for (i = 0; i < num_statements; i++)
    {
      ret =
	sqlite3_prepare_v2 (datbase_handle, statements[i], -1, &createStmt,
			    NULL);
      if (ret != SQLITE_OK)
	{
	  PRINT_ERROR ("Failed to prepare: %s\n",
		       sqlite3_errmsg (datbase_handle));
	  return ATMOTUBE_RET_ERROR;
	}

      ret = sqlite3_step (createStmt);
      if (ret != SQLITE_DONE)
	{
	  PRINT_ERROR ("Failed to create tables in %s\n", filename);
	  return ATMOTUBE_RET_ERROR;
	}

      PRINT_DEBUG ("Execute %d: %s, ret=%d\n", i, statements[i], ret);
      sqlite3_finalize (createStmt);
    }

  PRINT_DEBUG ("Created tables (%d)\n", ret);
  return ATMOTUBE_RET_OK;
}

int
db_plugin_create_statements (void)
{
  int ret =
    sqlite3_prepare_v2 (datbase_handle,
			"INSERT INTO device VALUES (NULL, ?1, ?2);", -1,
			&sql_statements[SQLS_INSERT_DEVICE], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: insert into device");
  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"select id from device where name=?1 and address=?2;",
			-1, &sql_statements[SQLS_SELECT_DEVICE], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: select from device");
  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"INSERT INTO `temperature` (device_id,time,value) VALUES (?1,?2,?3);",
			-1, &sql_statements[SQLS_INSERT_TEMP], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: insert into temperature");
  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"INSERT INTO `humidity` (device_id,time,value) VALUES (?1,?2,?3);",
			-1, &sql_statements[SQLS_INSERT_HUM], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: insert into humidity");
  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"INSERT INTO `voc` (device_id,time,value,description) VALUES (?1,?2,?3,?4);",
			-1, &sql_statements[SQLS_INSERT_VOC], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: insert into voc");

  /* Used for testing. */
  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"select value from temperature where device_id=?1 and time=?2;",
			-1, &sql_statements[SQLS_GET_TEMP], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: select temperature");

  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"select value from humidity where device_id=?1 and time=?2;",
			-1, &sql_statements[SQLS_GET_HUM], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: select hum");

  ret =
    sqlite3_prepare_v2 (datbase_handle,
			"select value from voc where device_id=?1 and time=?2;",
			-1, &sql_statements[SQLS_GET_VOC], NULL);
  RETURN_ATM_ERROR (ret, "Error creating: select voc");

  return ATMOTUBE_RET_OK;
}

void
db_plugin_destroy_statements (void)
{
  for (uint16_t i = 0; i < SQLS_MAX; i++)
    {
      if (sql_statements[i] != NULL)
	{
	  sqlite3_finalize (sql_statements[i]);
	}
    }
}

/* Get the row ID of the device described by name / address. */
int
db_plugin_find_device (const char *name, const char *address, int *id)
{
  sqlite3_reset (sql_statements[SQLS_SELECT_DEVICE]);
  int ret =
    sqlite3_bind_text (sql_statements[SQLS_SELECT_DEVICE], 1, name, -1,
		       SQLITE_STATIC);
  ret =
    sqlite3_bind_text (sql_statements[SQLS_SELECT_DEVICE], 2, address, -1,
		       SQLITE_STATIC);

  while ((ret =
	  sqlite3_step (sql_statements[SQLS_SELECT_DEVICE])) == SQLITE_ROW)
    {
      *id = sqlite3_column_int (sql_statements[SQLS_SELECT_DEVICE], 0);
      PRINT_DEBUG ("Found device %s/%s with id = %d\n", name, address, *id);
      return ATMOTUBE_RET_OK;
    }

  PRINT_ERROR ("Device %s/%s not found\n", name, address);
  return ATMOTUBE_RET_ERROR;
}

int
db_plugin_insert_device (const char *name, const char *address)
{
  int ret = 0;
  int id = -1;
  if (db_plugin_find_device (name, address, &id) == ATMOTUBE_RET_OK)
    {
      /* Device already exists. */
      return ATMOTUBE_RET_OK;
    }

  sqlite3_reset (sql_statements[SQLS_INSERT_DEVICE]);

  ret =
    sqlite3_bind_text (sql_statements[SQLS_INSERT_DEVICE], 1, name, -1,
		       SQLITE_STATIC);
  if (ret != SQLITE_OK)
    {
      PRINT_ERROR ("ERROR binding '%s': %s\n", name,
		   sqlite3_errmsg (datbase_handle));
      return ATMOTUBE_RET_ERROR;
    }

  ret =
    sqlite3_bind_text (sql_statements[SQLS_INSERT_DEVICE], 2, address, -1,
		       SQLITE_STATIC);
  if (ret != SQLITE_OK)
    {
      PRINT_ERROR ("ERROR binding '%s': %s\n", address,
		   sqlite3_errmsg (datbase_handle));
      return ATMOTUBE_RET_ERROR;
    }

  ret = sqlite3_step (sql_statements[SQLS_INSERT_DEVICE]);
  if (ret != SQLITE_DONE)
    {
      PRINT_ERROR ("ERROR inserting data: %s\n",
		   sqlite3_errmsg (datbase_handle));
      return ATMOTUBE_RET_ERROR;
    }

  PRINT_ERROR ("Inserted device %s/%s\n", name, address);
  return ATMOTUBE_RET_OK;
}

int
db_plugin_setup_database (const char *file_name,
			  const char *device_name, const char *device_address)
{
  PRINT_DEBUG ("Opening SQLite3 DB from string: %s,\n", file_name);
  PRINT_DEBUG ("using device '%s' with address '%s'\n", device_name,
	       device_address);

  filename = file_name;

  int ret = sqlite3_initialize ();
  if (ret != SQLITE_OK)
    {
      PRINT_ERROR ("Failed to init DB: %s\n", filename);
      return ATMOTUBE_RET_ERROR;
    }

  ret =
    sqlite3_open_v2 (filename, &datbase_handle,
		     SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE |
		     SQLITE_OPEN_CREATE, NULL);
  PRINT_DEBUG ("sqlite3_open_v2, ret = %d\n", ret);
  if (ret != SQLITE_OK)
    {
      PRINT_ERROR ("Failed to open DB: %s\n", filename);
      return ATMOTUBE_RET_ERROR;
    }

  return ATMOTUBE_RET_OK;
}

int
plugin_start (AtmotubeOutput * o)
{
  int ret = db_plugin_setup_database (o->filename,
				      o->device_name,
				      o->device_address);
  if (ret != ATMOTUBE_RET_OK)
    {
      return ATMOTUBE_RET_ERROR;
    }

  ret = db_plugin_create_tables ();
  if (ret != ATMOTUBE_RET_OK)
    {
      return ATMOTUBE_RET_ERROR;
    }

  ret = db_plugin_create_statements ();
  if (ret != ATMOTUBE_RET_OK)
    {
      return ATMOTUBE_RET_ERROR;
    }

  db_plugin_insert_device (o->device_name, o->device_address);
  /* Disregard error code here. */

  if (db_plugin_find_device
      (o->device_name, o->device_address, &device_row_id) != ATMOTUBE_RET_OK)
    {
      PRINT_ERROR ("DB, failed to find device '%s' with address '%s'\n",
		   o->device_name, o->device_address);
      return ATMOTUBE_RET_ERROR;
    }

  started = true;
  return ATMOTUBE_RET_OK;
}

int
plugin_stop (void)
{
  db_plugin_destroy_statements ();

  int ret = sqlite3_close (datbase_handle);
  if (ret != SQLITE_OK)
    {
      PRINT_ERROR ("Failed to close DB %s, reason:%s\n",
		   filename, sqlite3_errmsg (datbase_handle));
      return ATMOTUBE_RET_ERROR;
    }

  return ATMOTUBE_RET_OK;
}

int
get_temperature (unsigned long ts, unsigned long *value)
{
  int ret = sqlite3_reset (sql_statements[SQLS_GET_TEMP]);
  ret = sqlite3_bind_int64 (sql_statements[SQLS_GET_TEMP], 1, device_row_id);
  ret = sqlite3_bind_int64 (sql_statements[SQLS_GET_TEMP], 2, ts);

  while ((ret = sqlite3_step (sql_statements[SQLS_GET_TEMP])) == SQLITE_ROW)
    {
      *value = sqlite3_column_int64 (sql_statements[SQLS_GET_TEMP], 0);
      return ATMOTUBE_RET_OK;
    }
  return ATMOTUBE_RET_ERROR;
}

void
temperature (unsigned long ts, unsigned long value)
{
  PRINT_DEBUG ("Writing temperature to db(%u): %lu,%lu\n", started, ts,
	       value);

  if (started)
    {
      sqlite3_reset (sql_statements[SQLS_INSERT_TEMP]);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_TEMP], 1, device_row_id);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_TEMP], 2, ts);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_TEMP], 3, value);
      int ret = sqlite3_step (sql_statements[SQLS_INSERT_TEMP]);
      if (ret != SQLITE_DONE)
	{
	  PRINT_ERROR ("ERROR inserting data: %s\n",
		       sqlite3_errmsg (datbase_handle));
	}
    }
}

int
get_humidity (unsigned long ts, unsigned long *value)
{
  int ret = sqlite3_reset (sql_statements[SQLS_GET_HUM]);
  ret = sqlite3_bind_int64 (sql_statements[SQLS_GET_HUM], 1, device_row_id);
  ret = sqlite3_bind_int64 (sql_statements[SQLS_GET_HUM], 2, ts);

  while ((ret = sqlite3_step (sql_statements[SQLS_GET_HUM])) == SQLITE_ROW)
    {
      *value = sqlite3_column_int64 (sql_statements[SQLS_GET_HUM], 0);
      return ATMOTUBE_RET_OK;
    }
  return ATMOTUBE_RET_ERROR;
}

void
humidity (unsigned long ts, unsigned long value)
{
  PRINT_DEBUG ("Writing humidity to db(%u): %lu,%lu\n", started, ts, value);
  if (started)
    {
      sqlite3_reset (sql_statements[SQLS_INSERT_HUM]);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_HUM], 1, device_row_id);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_HUM], 2, ts);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_HUM], 3, value);
      int ret = sqlite3_step (sql_statements[SQLS_INSERT_HUM]);
      if (ret != SQLITE_DONE)
	{
	  PRINT_ERROR ("ERROR inserting data: %s\n",
		       sqlite3_errmsg (datbase_handle));
	}
    }
}

int
get_voc (unsigned long ts, float *value)
{
  int ret = sqlite3_reset (sql_statements[SQLS_GET_VOC]);
  ret = sqlite3_bind_int64 (sql_statements[SQLS_GET_VOC], 1, device_row_id);
  ret = sqlite3_bind_int64 (sql_statements[SQLS_GET_VOC], 2, ts);

  while ((ret = sqlite3_step (sql_statements[SQLS_GET_VOC])) == SQLITE_ROW)
    {
      *value = sqlite3_column_double (sql_statements[SQLS_GET_VOC], 0);
      return ATMOTUBE_RET_OK;
    }
  return ATMOTUBE_RET_ERROR;
}

void
voc (unsigned long ts, float value)
{
  PRINT_DEBUG ("Writing voc to db(%u): %lu,%f\n", started, ts, value);
  if (started)
    {
      sqlite3_reset (sql_statements[SQLS_INSERT_VOC]);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_VOC], 1, device_row_id);
      sqlite3_bind_int64 (sql_statements[SQLS_INSERT_VOC], 2, ts);
      sqlite3_bind_double (sql_statements[SQLS_INSERT_VOC], 3, value);
      sqlite3_bind_text (sql_statements[SQLS_INSERT_VOC], 4, "", -1,
			 SQLITE_STATIC);

      int ret = sqlite3_step (sql_statements[SQLS_INSERT_VOC]);
      if (ret != SQLITE_DONE)
	{
	  PRINT_ERROR ("ERROR inserting data: %s\n",
		       sqlite3_errmsg (datbase_handle));
	}
    }
}
