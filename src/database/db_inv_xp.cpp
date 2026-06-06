// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <ctime>
#include <sqlite3.h>

#include "db.h"

namespace db {

	void inv_add_timed(dpp::snowflake g_id, dpp::snowflake u_id, int item_id, long expires) {
		sqlite3_stmt* s;
		const char* sql = "INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
						  " VALUES (?, ?, ?, ?, ?, 1)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_bind_int64(s, 4, std::time(nullptr));
			sqlite3_bind_int64(s, 5, expires);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	double inv_xp_mult(dpp::snowflake g_id, dpp::snowflake u_id) {
		inv_purge(g_id, u_id);
		sqlite3_stmt* s;
		double max_mult = 1.0;
		long now = std::time(nullptr);
		const char* sql = "SELECT s.data FROM inventory i JOIN shop_items s ON i.item_id = s.item_id"
						  " WHERE i.guild_id = ? AND i.user_id = ? AND i.equipped = 1 AND s.type = 'xp_boost'"
						  " AND (i.expires = 0 OR i.expires > ?)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(s, 3, now);
			while (sqlite3_step(s) == SQLITE_ROW) {
				const unsigned char* data = sqlite3_column_text(s, 0);
				if (!data) continue;
				std::string raw = reinterpret_cast<const char*>(data);
				auto pos = raw.find("\"mult\"");
				if (pos != std::string::npos) {
					auto colon = raw.find(':', pos);
					if (colon != std::string::npos) {
						try {
							double m = std::stod(raw.substr(colon + 1));
							if (m > max_mult) max_mult = m;
						} catch (...) {}
					}
				}
			}
			sqlite3_finalize(s);
		}
		return max_mult;
	}

} // namespace db
