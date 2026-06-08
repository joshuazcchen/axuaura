// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <iostream>
#include <sqlite3.h>

#include "db.h"

namespace db {

	struct Migrate {
		int version;
		const char* sql;
	};

	static const Migrate migrations[] = {
		{1, "ALTER TABLE shop_items ADD COLUMN pinned INTEGER DEFAULT 0;"},
		{2, "ALTER TABLE shop_items ADD COLUMN global INTEGER DEFAULT 0;"},
		{3, "CREATE TABLE IF NOT EXISTS bazaar_rotation ("
			"guild_id TEXT NOT NULL, slot INTEGER NOT NULL, item_id INTEGER NOT NULL, "
			"refreshed_at INTEGER NOT NULL, PRIMARY KEY (guild_id, slot));"},
		{4, "UPDATE shop_items SET DATA = '{}' WHERE data IS NULL OR data = '' OR data = 'null';"},
		{5, "CREATE TABLE IF NOT EXISTS user_badges ("
			"user_id TEXT NOT NULL, badge_id TEXT NOT NULL, granted_at INTEGER NOT NULL, "
			"PRIMARY KEY (user_id, badge_id));"
			"CREATE TABLE IF NOT EXISTS global_boosts ("
			"user_id TEXT PRIMARY KEY, mult REAL NOT NULL, expires INTEGER NOT NULL);"},
		{6, "CREATE TABLE IF NOT EXISTS starboard_channels ("
			"guild_id TEXT NOT NULL, channel_id TEXT NOT NULL, "
			"PRIMARY KEY (guild_id, channel_id));"},
		{7, "CREATE TABLE IF NOT EXISTS global_banners ("
			"user_id TEXT PRIMARY KEY, filename TEXT NOT NULL, "
			"artist TEXT DEFAULT '', invert INTEGER DEFAULT 0);"},
		{0, nullptr},
	};

	void db_migrate() {
		int cur = 0;
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, "SELECT MAX(version) FROM db_version;", -1, &s, nullptr) == SQLITE_OK) {
			if (sqlite3_step(s) == SQLITE_ROW && sqlite3_column_type(s, 0) != SQLITE_NULL)
				cur = sqlite3_column_int(s, 0);
			sqlite3_finalize(s);
		}

		for (int i = 0; migrations[i].sql; ++i) {
			if (migrations[i].version <= cur) continue;
			sqlite3_exec(db_ptr, migrations[i].sql, nullptr, nullptr, nullptr);
			if (sqlite3_prepare_v2(db_ptr, "INSERT OR IGNORE INTO db_version (version, updated) VALUES (?, ?);", -1, &s,
								   nullptr) == SQLITE_OK) {
				sqlite3_bind_int(s, 1, migrations[i].version);
				sqlite3_bind_int64(s, 2, std::time(nullptr));
				sqlite3_step(s);
				sqlite3_finalize(s);
			}
		}
	}

} // namespace db
