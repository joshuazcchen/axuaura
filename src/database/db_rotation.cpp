// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <ctime>
#include <sqlite3.h>

#include "db.h"

namespace db {

	std::vector<RotationSlot> bazaar_rotation_get(dpp::snowflake g_id) {
		std::vector<RotationSlot> slots;
		sqlite3_stmt* s;
		const char* sql = "SELECT slot, item_id, refreshed_at FROM bazaar_rotation"
						  " WHERE guild_id = ? ORDER BY slot";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW) {
				RotationSlot r;
				r.slot = sqlite3_column_int(s, 0);
				r.item_id = sqlite3_column_int(s, 1);
				r.refreshed_at = sqlite3_column_int64(s, 2);
				slots.push_back(r);
			}
			sqlite3_finalize(s);
		}
		return slots;
	}

	void bazaar_rotation_set(dpp::snowflake g_id, int slot, int item_id) {
		sqlite3_stmt* s;
		const char* sql = "INSERT INTO bazaar_rotation (guild_id, slot, item_id, refreshed_at) VALUES (?, ?, ?, ?)"
						  " ON CONFLICT(guild_id, slot) DO UPDATE SET item_id = ?, refreshed_at = ?";
		long now = std::time(nullptr);
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, slot);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_bind_int64(s, 4, now);
			sqlite3_bind_int(s, 5, item_id);
			sqlite3_bind_int64(s, 6, now);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void bazaar_rotation_clear_slot(dpp::snowflake g_id, int slot) {
		sqlite3_stmt* s;
		const char* sql = "DELETE FROM bazaar_rotation WHERE guild_id = ? AND slot = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, slot);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void bazaar_rotation_clear_all(dpp::snowflake g_id) {
		sqlite3_stmt* s;
		const char* sql = "DELETE FROM bazaar_rotation WHERE guild_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

} // namespace db
