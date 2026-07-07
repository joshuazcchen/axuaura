// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <sqlite3.h>

#include <ctime>
#include <string>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	void d_issue(dpp::snowflake guild_id, dpp::snowflake challenger, dpp::snowflake target, int bet) {
		std::string g_id_str = std::to_string(guild_id);
		std::string c_str = std::to_string(challenger);
		std::string t_str = std::to_string(target);
		long issue_time = std::time(nullptr);
		const char* sql = "INSERT INTO duels (guild_id, challenger, target, bet, issue_time) VALUES (?, ?, ?, ?, ?) "
						  "ON CONFLICT(guild_id, challenger) DO UPDATE SET target = ?, bet = ?, issue_time = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, c_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, t_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 4, bet);
			sqlite3_bind_int64(stmt, 5, issue_time);
			sqlite3_bind_text(stmt, 6, t_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 7, bet);
			sqlite3_bind_int64(stmt, 8, issue_time);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	int d_check(dpp::snowflake guild_id, dpp::snowflake challenger, dpp::snowflake target) {
		std::string g_id_str = std::to_string(guild_id);
		std::string c_str = std::to_string(challenger);
		std::string t_str = std::to_string(target);
		const char* sql = "SELECT bet, issue_time FROM duels WHERE guild_id = ? AND challenger = ? AND target = ?;";
		sqlite3_stmt* stmt;
		int bet = -1;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, c_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, t_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				bet = sqlite3_column_int(stmt, 0);
				long issue_time = sqlite3_column_int64(stmt, 1);
				if (std::time(nullptr) - issue_time >= 120) { bet = -1; }
			}
		}
		sqlite3_finalize(stmt);
		if (bet == -1) { d_delete(guild_id, challenger); }
		return bet;
	}

	long d_time(dpp::snowflake guild_id, dpp::snowflake challenger) {
		std::string g_id_str = std::to_string(guild_id);
		std::string c_str = std::to_string(challenger);
		const char* sql = "SELECT issue_time FROM duels WHERE guild_id = ? AND challenger = ?;";
		sqlite3_stmt* stmt;
		long time = -1;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, c_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) { time = sqlite3_column_int64(stmt, 0); }
		}
		sqlite3_finalize(stmt);
		return time;
	}

	void d_delete(dpp::snowflake guild_id, dpp::snowflake challenger) {
		std::string g_id_str = std::to_string(guild_id);
		std::string c_str = std::to_string(challenger);
		const char* sql = "DELETE FROM duels WHERE guild_id = ? AND challenger = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, c_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	bool d_outgoing(dpp::snowflake guild_id, dpp::snowflake challenger) {
		std::string g_id_str = std::to_string(guild_id);
		std::string c_str = std::to_string(challenger);
		const char* sql = "SELECT 1 FROM duels WHERE guild_id = ? AND challenger = ?;";
		sqlite3_stmt* stmt;
		bool yes = false;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, c_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) { yes = true; }
		}
		sqlite3_finalize(stmt);
		return yes;
	}
} // namespace db
