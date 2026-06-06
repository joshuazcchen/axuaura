// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <sqlite3.h>

#include <string>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	long vc_get(dpp::snowflake guild_id, dpp::snowflake user_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "SELECT join_time FROM voice WHERE guild_id = ? AND user_id = ?;";
		sqlite3_stmt* stmt;
		long time = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) time = sqlite3_column_int64(stmt, 0);
		}
		sqlite3_finalize(stmt);
		return time;
	}

	void vc_set(dpp::snowflake guild_id, dpp::snowflake user_id, long time) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "INSERT INTO voice (guild_id, user_id, join_time) VALUES (?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET join_time = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(stmt, 3, time);
			sqlite3_bind_int64(stmt, 4, time);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void vc_clr(dpp::snowflake guild_id, dpp::snowflake user_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "DELETE FROM voice WHERE guild_id = ? AND user_id = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}
} // namespace db
