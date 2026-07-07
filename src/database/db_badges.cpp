// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <ctime>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "db.h"

namespace db {

	void badge_grant(dpp::snowflake user_id, const std::string& badge_id) {
		const char* sql = "INSERT OR IGNORE INTO user_badges (user_id, badge_id, granted_at) VALUES (?, ?, ?);";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, badge_id.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(s, 3, std::time(nullptr));
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void badge_revoke(dpp::snowflake user_id, const std::string& badge_id) {
		const char* sql = "DELETE FROM user_badges WHERE user_id = ? AND badge_id = ?;";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, badge_id.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	std::vector<std::string> badge_get(dpp::snowflake user_id) {
		std::vector<std::string> result;
		const char* sql = "SELECT badge_id FROM user_badges WHERE user_id = ? ORDER BY granted_at ASC LIMIT 12;";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW) {
				result.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(s, 0)));
			}
			sqlite3_finalize(s);
		}
		return result;
	}

} // namespace db
