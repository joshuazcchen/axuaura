#include <iostream>
#include <sqlite3.h>

#include "db.h"

namespace db {

	void db_migrate() {
		const char* alters[] = {"ALTER TABLE shop_items ADD COLUMN pinned INTEGER DEFAULT 0;",
								"ALTER TABLE shop_items ADD COLUMN global INTEGER DEFAULT 0;",
								"CREATE TABLE IF NOT EXISTS user_badges (",
								"user_id TEXT NOT NULL, badge_id TEXT NOT NULL, granted_at INTEGER NOT NULL, ",
								"PRIMARY KEY (user_id, badge_id));",
								"CREATE TABLE IF NOT EXISTS global_boosts (",
								"user_id TEXT PRIMARY KEY, mult REAL NOT NULL, expires INTEGER NOT NULL);",
								nullptr};
		for (int i = 0; alters[i]; ++i)
			sqlite3_exec(db_ptr, alters[i], nullptr, nullptr, nullptr);

		sqlite3_exec(db_ptr,
					 "CREATE TABLE IF NOT EXISTS bazaar_rotation ("
					 "guild_id TEXT NOT NULL, slot INTEGER NOT NULL, item_id INTEGER NOT NULL"
					 "refreshed_at INTEGER NOT NULL, PRIMARY KEY (guild_id, slot));",
					 nullptr, nullptr, nullptr);

		sqlite3_exec(db_ptr,
					 "UPDATE shop_items SET data = '{}'"
					 "WHERE data IS NULL OR data = '' OR data = 'null';",
					 nullptr, nullptr, nullptr);
	}

} // namespace db
