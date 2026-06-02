#include <sqlite3.h>

#include <string>
#include <vector>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	int xp_get(dpp::snowflake guild_id, dpp::snowflake user_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "SELECT xp FROM xp WHERE guild_id = ? AND user_id = ?;";
		sqlite3_stmt* stmt;
		int xp = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) xp = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);
		return xp;
	}

	int lvl_get(dpp::snowflake guild_id, dpp::snowflake user_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "SELECT level FROM xp WHERE guild_id = ? AND user_id = ?;";
		sqlite3_stmt* stmt;
		int lvl = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) lvl = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);
		return lvl;
	}

	void xp_add(dpp::snowflake guild_id, dpp::snowflake user_id, int amt) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "INSERT INTO xp (guild_id, user_id, xp) VALUES (?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET xp = xp + ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 3, amt);
			sqlite3_bind_int(stmt, 4, amt);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void xp_lvl_set(dpp::snowflake guild_id, dpp::snowflake user_id, int xp, int lvl) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "INSERT INTO xp (guild_id, user_id, xp, level) VALUES (?, ?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET xp = ?, level = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 3, xp);
			sqlite3_bind_int(stmt, 4, lvl);
			sqlite3_bind_int(stmt, 5, xp);
			sqlite3_bind_int(stmt, 6, lvl);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	long xp_time_get(dpp::snowflake guild_id, dpp::snowflake user_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "SELECT xp_time FROM xp WHERE guild_id = ? AND user_id = ?;";
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

	void xp_time_set(dpp::snowflake guild_id, dpp::snowflake user_id, long time) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "INSERT INTO xp (guild_id, user_id, xp_time) VALUES (?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET xp_time = ?;";
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

	std::vector<std::pair<dpp::snowflake, int>> xp_top(dpp::snowflake guild_id, int limit) {
		std::vector<std::pair<dpp::snowflake, int>> result;
		std::string g_id_str = std::to_string(guild_id);
		const char* sql = "SELECT user_id, xp FROM xp WHERE guild_id = ? AND xp > 0 ORDER BY xp DESC LIMIT ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, limit);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				dpp::snowflake uid = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
				int xp = sqlite3_column_int(stmt, 1);
				result.push_back({uid, xp});
			}
		}
		sqlite3_finalize(stmt);
		return result;
	}

	void xp_set(dpp::snowflake guild_id, dpp::snowflake user_id, int xp, int level) {
		const char* sql = "INSERT INTO xp (guild_id, user_id, xp, level) VALUES (/, ?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET xp = ?, level = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 3, xp);
			sqlite3_bind_int(stmt, 4, level);
			sqlite3_bind_int(stmt, 5, xp);
			sqlite3_bind_int(stmt, 6, level);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}
	}
} // namespace db
