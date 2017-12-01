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

/* Statements used by this plugin. */
static sqlite3_stmt *stmt_insert_device = NULL;
static sqlite3_stmt *stmt_select_device = NULL;

const char* get_plugin_type(void)
{
    return type;
}

static int create_statements() {
    sqlite3_prepare_v2(datbase_handle, "INSERT INTO `device` (name,address) VALUES ('?1','?2');", -1, &stmt_insert_device, NULL);
    sqlite3_prepare_v2(datbase_handle, "select rowid from device where name='?1' and address='?2';", -1, &stmt_select_device, NULL);
    
    return ATMOTUBE_RET_OK;
}

static int create_tables()
{
    const char* statements[] = {
	"CREATE TABLE IF NOT EXISTS `voc` ( \
	`time`	INTEGER NOT NULL,	    \
	`value`	REAL NOT NULL,		    \
	`description` TEXT NOT NULL);",
	/* */
	"CREATE TABLE IF NOT EXISTS `temperature` (	\
	`time`	NUMERIC NOT NULL,			\
	`value`	INTEGER NOT NULL);",
	/* */
	"CREATE TABLE IF NOT EXISTS `device` (	\
	`name`	TEXT NOT NULL, \
	`address` TEXT NOT NULL);",
	/* */
	"CREATE TABLE IF NOT EXISTS `humidity` ( \
	`time`	INTEGER NOT NULL,		 \
	`value`	INTEGER NOT NULL,		 \
	PRIMARY KEY(`time`) );",
	/* */
	"CREATE INDEX IF NOT EXISTS `voc_index` ON `voc` ( \
	`time`	ASC,					   \
	`value`	ASC,					   \
	`description`	ASC);",
	/* */
	"CREATE UNIQUE INDEX IF NOT EXISTS `temperature_index` ON `temperature` ( \
	`time`	ASC, \
	`value`	ASC);",
	/* */
	"CREATE UNIQUE INDEX IF NOT EXISTS `humidity_index` ON `humidity` ( \
	`time`	ASC,							\
	`value`	ASC);",
	/* */
	"CREATE UNIQUE INDEX IF NOT EXISTS `device_index` ON `device` ( \
	`name`	ASC, \
	`address` ASC);"
    };

    uint16_t num_statements = sizeof(statements) / sizeof(char*);
    sqlite3_stmt *createStmt;

    int i = 0;
    int ret = 0;
    for (i = 0; i < num_statements; i++) {
	PRINT_DEBUG("Execute %d: %s\n", i, statements[i]);

	ret = sqlite3_prepare_v2(datbase_handle, statements[i], -1, &createStmt, NULL);
	if (ret != SQLITE_OK) {
	    PRINT_ERROR("Failed to prepare: %s\n", sqlite3_errmsg(datbase_handle));
	    return ATMOTUBE_RET_ERROR;
	}

	ret = sqlite3_step(createStmt);
	if (ret != SQLITE_DONE) {
	    PRINT_ERROR("Failed to create tables in %s\n", filename);
	    return ATMOTUBE_RET_ERROR;
	}

	sqlite3_finalize(createStmt);
    }
    
    PRINT_DEBUG("Created tables (%d)\n", ret);
    return ATMOTUBE_RET_OK;
}

/* Get the row ID of the device described by name / address. */
static int find_device(const char *name, const char *address, int *id)
{
    int ret;
    sqlite3_bind_text(stmt_select_device, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_select_device, 2, address, -1, SQLITE_STATIC);
    ret = sqlite3_step(stmt_select_device);

    if (ret != SQLITE_DONE) {
	PRINT_ERROR("ERROR selecting data: %s\n", sqlite3_errmsg(datbase_handle));
	return ATMOTUBE_RET_ERROR;
    }

    while ( (ret = sqlite3_step(stmt_select_device)) == SQLITE_ROW) {
	*id = sqlite3_column_int(stmt_select_device, 1);
	return ATMOTUBE_RET_OK;
    }
    return ATMOTUBE_RET_ERROR;
}

static int insert_device(char *name, char* address)
{
    /* Insert device */
    /* INSERT INTO `device` (name,address) VALUES ('1111','00:1A:7D:1E:2E:42'); */

    sqlite3_bind_text(stmt_insert_device, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_device, 2, address, -1, SQLITE_STATIC);

    int ret = sqlite3_step(stmt_insert_device);
    if (ret != SQLITE_DONE) {
	PRINT_ERROR("ERROR inserting data: %s\n", sqlite3_errmsg(datbase_handle));
    }

    return ATMOTUBE_RET_OK;
}

int plugin_start(AtmotubeOutput* o)
{
    PRINT_DEBUG("Opening SQLite3 DB from string: %s with\n", o->filename);
    filename = o->filename;

    int ret = sqlite3_initialize();
    if (ret != SQLITE_OK) {
	PRINT_ERROR("Failed to init DB: %s\n", filename);    
	return ATMOTUBE_RET_ERROR;
    }
    
    ret = sqlite3_open_v2(filename, &datbase_handle, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    PRINT_DEBUG("sqlite3_open_v2, ret = %d\n", ret);
    if (ret != SQLITE_OK) {
	PRINT_ERROR("Failed to open DB: %s\n", filename);    
	return ATMOTUBE_RET_ERROR;
    }

    ret = create_tables();
    if (ret != ATMOTUBE_RET_OK) {
	return ATMOTUBE_RET_ERROR;
    }

    ret = create_statements();
    if (ret != ATMOTUBE_RET_OK) {
	return ATMOTUBE_RET_ERROR;
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
