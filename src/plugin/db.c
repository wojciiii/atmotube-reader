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
static const char* type = "db";
static sqlite3 *datbase_handle = NULL;
static const char *filename = NULL;

static sqlite3_stmt *insert_device;

const char* get_plugin_type(void)
{
    return type;
}

static int create_tables()
{
    const char* create_table_sql = "BEGIN TRANSACTION; \
CREATE TABLE IF NOT EXISTS `voc` ( \
	`time`	INTEGER NOT NULL, \
	`value`	REAL NOT NULL, \
	`description`	TEXT NOT NULL \
); \
CREATE TABLE IF NOT EXISTS `temperature` ( \
	`time`	NUMERIC NOT NULL, \
	`value`	INTEGER NOT NULL \
); \
CREATE TABLE IF NOT EXISTS `device` ( \
	`name`	TEXT NOT NULL, \
	`address`	TEXT NOT NULL \
); \
CREATE TABLE IF NOT EXISTS `humidity` ( \
	`time`	INTEGER NOT NULL, \
	`value`	INTEGER NOT NULL, \
	PRIMARY KEY(`time`) \
); \
CREATE INDEX IF NOT EXISTS `voc_index` ON `voc` ( \
	`time`	ASC, \
	`value`	ASC, \
	`description`	ASC \
); \
CREATE UNIQUE INDEX IF NOT EXISTS `temperature_index` ON `temperature` ( \
	`time`	ASC, \
	`value`	ASC \
); \
CREATE UNIQUE INDEX IF NOT EXISTS `humidity_index` ON `humidity` ( \
	`time`	ASC, \
	`value`	ASC \
); \
CREATE UNIQUE INDEX IF NOT EXISTS `device_index` ON `device` ( \
	`name`	ASC, \
	`address` ASC \
); \
COMMIT; \
";
    sqlite3_stmt *createStmt;

    int ret = sqlite3_prepare_v2(datbase_handle, create_table_sql, -1, &createStmt, NULL);
    if (ret != SQLITE_OK) {
	PRINT_ERROR("Failed to prepare: %s\n", sqlite3_errmsg(datbase_handle));
	return ATMOTUBE_RET_ERROR;
    }
    
    if (sqlite3_step(createStmt) != SQLITE_DONE) {
	PRINT_ERROR("Failed to create tables in %s\n", filename);
	return ATMOTUBE_RET_ERROR;
    }

    sqlite3_finalize(createStmt);
    
    PRINT_DEBUG("Created tables\n");
    return ATMOTUBE_RET_OK;
}

int plugin_start(AtmotubeOutput* o)
{
    PRINT_DEBUG("Opening SQLite3 DB from string: %s with\n", o->filename);
    filename = o->filename;

    int ret = sqlite3_open_v2(o->filename, &datbase_handle, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    PRINT_DEBUG("sqlite3_open_v2, ret = %d\n", ret);
    if (ret != SQLITE_OK) {
	PRINT_ERROR("Failed to open DB: %s\n", o->filename);    
	return ATMOTUBE_RET_ERROR;
    }

    ret = create_tables();
    if (ret != ATMOTUBE_RET_OK) {
	return ATMOTUBE_RET_ERROR;
    }

    /* Insert device */
    /* INSERT INTO `device` (name,address) VALUES ('1111','00:1A:7D:1E:2E:42'); */

    sqlite3_prepare_v2(datbase_handle, "INSERT INTO `device` (name,address) VALUES ('?1','?2');", -1, &insert_device, NULL);

    sqlite3_bind_text(insert_device, 1, "test device", -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_device, 2, "00:1A:7D:1E:2E:42", -1, SQLITE_STATIC);

    ret = sqlite3_step(insert_device);
    if (ret != SQLITE_DONE) {
	PRINT_ERROR("ERROR inserting data: %s\n", sqlite3_errmsg(datbase_handle));
    }
    
    started = true;
    return ATMOTUBE_RET_OK;
}

int plugin_stop(void)
{
    int ret = sqlite3_close(datbase_handle);
    if (ret != SQLITE_OK) {
	PRINT_ERROR("Failed to close DB: %s\n", filename);    
	return ATMOTUBE_RET_ERROR;
    }

    return ATMOTUBE_RET_OK;
}

void temperature(unsigned long ts, unsigned long value)
{
    PRINT_DEBUG("Writing temperature to db(%u): %lu,%lu\n", started, ts, value);
    if (started) {
	/*
        string insertQuery = "INSERT INTO items (time, ipaddr,username,useradd,userphone,age) VALUES ('7:30', '192.187.27.55','vivekanand','kolkatta','04456823948',74);";
    sqlite3_stmt *insertStmt;
    cout << "Creating Insert Statement" << endl;
    sqlite3_prepare(db, insertQuery.c_str(), insertQuery.size(), &insertStmt, NULL);
    cout << "Stepping Insert Statement" << endl;
    if (sqlite3_step(insertStmt) != SQLITE_DONE) cout << "Didn't Insert Item!" << endl;
	*/

    }
}

void humidity(unsigned long ts, unsigned long value)
{
    PRINT_DEBUG("Writing humidity to file(%u): %lu,%lu\n", started, ts, value);
    if (started) {

    }
}

void voc(unsigned long ts, float value)
{
    PRINT_DEBUG("Writing voc to file(%u): %lu,%f\n", started, ts, value);
    if (started) {

    }
}
