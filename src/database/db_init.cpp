// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

// overhauling the entire database system into separate files so that I don't have to work with a monolith of like
// 50,000,000 lines every time I make any database changes.
//
// db_init.cpp: just has the init and the initial db calls

#include <sqlite3.h>

#include <iostream>
#include <vector>

#include "db.h"

namespace db {
	sqlite3* db_ptr = nullptr;

	void init() {
		const char* db_path_env = std::getenv("DATABASE_PATH");
		std::string db_path = db_path_env ? db_path_env : "database.sqlite";
		if (sqlite3_open(db_path.c_str(), &db_ptr) != SQLITE_OK) {
			std::cerr << "database failed to load: " << sqlite3_errmsg(db_ptr) << std::endl;
			return;
		}

		sqlite3_exec(db_ptr, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

		const char* db_version = "CREATE TABLE IF NOT EXISTS db_version ("
								 "version INTEGER PRIMARY KEY, "
								 "updated INTEGER"
								 ");";
		sqlite3_exec(db_ptr, db_version, nullptr, nullptr, nullptr);

		const char* v2_schema = "CREATE TABLE IF NOT EXISTS aura ("
								"guild_id TEXT NOT NULL, "
								"user_id TEXT NOT NULL, "
								"amount INTEGER DEFAULT 0, "
								"PRIMARY KEY (guild_id, user_id));"

								"CREATE TABLE IF NOT EXISTS xp ("
								"guild_id TEXT NOT NULL, user_id TEXT NOT NULL, xp INTEGER DEFAULT 0, "
								"level INTEGER DEFAULT 0, xp_time INTEGER DEFAULT 0, "
								"PRIMARY KEY (guild_id, user_id));"

								"CREATE TABLE IF NOT EXISTS guild_settings ("
								"guild_id TEXT NOT NULL, key TEXT NOT NULL, value TEXT, "
								"PRIMARY KEY (guild_id, key));"

								"CREATE TABLE IF NOT EXISTS polls ("
								"p_id INTEGER PRIMARY KEY AUTOINCREMENT, guild_id TEXT NOT NULL, "
								"title TEXT, ops TEXT, active INTEGER DEFAULT 1);"

								"CREATE TABLE IF NOT EXISTS bets ("
								"p_id INTEGER, u_id TEXT, op TEXT, amt INTEGER, "
								"FOREIGN KEY(p_id) REFERENCES polls(p_id));"

								"CREATE TABLE IF NOT EXISTS duels ("
								"guild_id TEXT NOT NULL, challenger TEXT NOT NULL, target TEXT, "
								"bet INTEGER, issue_time INTEGER, "
								"PRIMARY KEY (guild_id, challenger));"

								"CREATE TABLE IF NOT EXISTS voice ("
								"guild_id TEXT NOT NULL, user_id TEXT NOT NULL, join_time INTEGER, "
								"PRIMARY KEY (guild_id, user_id));"

								"CREATE TABLE IF NOT EXISTS shop_items ("
								"item_id INTEGER PRIMARY KEY AUTOINCREMENT, guild_id TEXT NOT NULL, "
								"type TEXT NOT NULL, role_id TEXT, name TEXT NOT NULL, desc TEXT, "
								"cost INTEGER NOT NULL, data TEXT, active INTEGER DEFAULT 1, "
								"obtainable INTEGER DEFAULT 1, sellability INTEGER DEFAULT 1, "
								"pinned INTEGER DEFAULT 0, global INTEGER DEFAULT 0);"

								"CREATE TABLE IF NOT EXISTS inventory ("
								"inv_id INTEGER PRIMARY KEY AUTOINCREMENT, guild_id TEXT NOT NULL, "
								"user_id TEXT NOT NULL, item_id INTEGER NOT NULL, "
								"acquired INTEGER NOT NULL, expires INTEGER, equipped INTEGER DEFAULT 0, "
								"FOREIGN KEY(item_id) REFERENCES shop_items(item_id));"

								"CREATE TABLE IF NOT EXISTS bazaar_rotation ("
								"guild_id TEXT NOT NULL, slot INTEGER NOT NULL, "
								"item_id INTEGER NOT NULL, refreshed_at INTEGER NOT NULL,"
								"PRIMARY KEY (guild_id, slot));"

								"CREATE TABLE IF NOT EXISTS user_badges ("
								"user_id TEXT NOT NULL, badge_id TEXT NOT NULL, granted_at INTEGER NOT NULL, "
								"PRIMARY KEY (user_id, badge_id));"

								"CREATE TABLE IF NOT EXISTS global_boosts ("
								"user_id TEXT PRIMARY KEY, mult REAL NOT NULL, expires INTEGER NOT NULL);";

		sqlite3_exec(db_ptr, v2_schema, nullptr, nullptr, nullptr);

		db::db_migrate();
	}
} // namespace db
