// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <ctime>
#include <iostream>
#include <sqlite3.h>
#include <string>

#include "axuctl.h"
#include "dotenv.h"

sqlite3* db = nullptr;

void db_open() {
	dotenv::init();
	const char* path = std::getenv("DATABASE_PATH");
	if (!path) path = "database.sqlite";
	if (sqlite3_open(path, &db) != SQLITE_OK) {
		std::cerr << "failed to open db: " << sqlite3_errmsg(db) << "\n";
		std::exit(1);
	}
	sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
	sqlite3_exec(db,
				 "CREATE TABLE IF NOT EXISTS user_badges ("
				 "user_id TEXT NOT NULL, badge_id TEXT NOT NULL, granted_at INTEGER NOT NULL, "
				 "PRIMARY KEY (user_id, badge_id));"
				 "CREATE TABLE IF NOT EXISTS gb_boosts ("
				 "user_id TEXT PRIMARY KEY, mult REAL NOT NULL, expires INTEGER NOT NULL);",
				 nullptr, nullptr, nullptr);
}

void usage() {
	std::cerr << "usage:\n"
			  << "axuctl badge grant <user_id> <badge_id>\n"
			  << "axuctl badge revoke <user_id> <badge_id>\n"
			  << "axuctl badge list <user_id>\n"
			  << "axuctl xpboost set <user_id> <mult> [hours] (0 = permanent)\n"
			  << "axuctl xpboost clear <user_id>\n"
			  << "axuctl xpboost info <user_id>\n"
			  << "axuctl bg global set <user_id> <bg_name> <artist> <invert(0|1)>\n"
			  << "axuctl bg global clear <user_id>\n"
			  << "axuctl bg global info <user_id>\n"
			  << "axuctl bg server set <guild_id> <user_id> <bg_name> <artist> <invert(0|1)>\n"
			  << "axuctl bg server clear <guild_id> <user_id>\n"
			  << "axuctl bg server info <guild_id> <user_id>\n"
			  << "axuctl guild set <guild_id> <key> <value>\n"
			  << "axuctl guild get <guild_id> <key>\n"
			  << "axuctl guild list <guild_id>\n";
}

// shared helper: bind up to two text params and step
void exec_sql(const char* sql, const char* a, const char* b) {
	sqlite3_stmt* s;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		if (a) sqlite3_bind_text(s, 1, a, -1, SQLITE_TRANSIENT);
		if (b) sqlite3_bind_text(s, 2, b, -1, SQLITE_TRANSIENT);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}
}

void setting_set(const char* guild_id, const char* key, const char* val) {
	const char* sql = "INSERT INTO guild_settings (guild_id, key, value) VALUES (?, ?, ?) "
					  "ON CONFLICT(guild_id, key) DO UPDATE SET value = ?;";
	sqlite3_stmt* s;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, guild_id, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 3, val, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 4, val, -1, SQLITE_TRANSIENT);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}
}

std::string setting_get(const char* guild_id, const char* key) {
	const char* sql = "SELECT value FROM guild_settings WHERE guild_id = ? AND key = ?;";
	sqlite3_stmt* s;
	std::string result;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, guild_id, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
		if (sqlite3_step(s) == SQLITE_ROW) {
			const unsigned char* t = sqlite3_column_text(s, 0);
			if (t) result = reinterpret_cast<const char*>(t);
		}
		sqlite3_finalize(s);
	}
	return result;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		usage();
		return 1;
	}
	db_open();

	std::string domain = argv[1];
	int ret = 1;

	if (domain == "badge")
		ret = cmd_badge(argc, argv);
	else if (domain == "xpboost")
		ret = cmd_xpboost(argc, argv);
	else if (domain == "bg")
		ret = cmd_bg(argc, argv);
	else if (domain == "guild")
		ret = cmd_guild(argc, argv);
	else { usage(); }

	sqlite3_close(db);
	return ret;
}
