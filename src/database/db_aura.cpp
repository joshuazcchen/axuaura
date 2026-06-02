#include <sqlite3.h>

#include <string>
#include <vector>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	// TODO: convert this into standard naming scheme of aura_(thing)
	int get_aura(dpp::snowflake guild_id, dpp::snowflake user_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "SELECT amount FROM aura WHERE guild_id = ? AND user_id = ?;";
		sqlite3_stmt* stmt;
		int aura = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) { aura = sqlite3_column_int(stmt, 0); }
		}
		sqlite3_finalize(stmt);
		return aura;
	}

	std::vector<std::pair<std::string, int>> get_ab(dpp::snowflake guild_id, int limit, bool bottom) {
		std::vector<std::pair<std::string, int>> results;
		std::string g_id_str = std::to_string(guild_id);
		std::string order = bottom ? "ASC" : "DESC";
		std::string sql = "SELECT user_id, amount FROM aura WHERE guild_id = ? ORDER BY amount " + order + " LIMIT ?;";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, limit);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
				int aura = sqlite3_column_int(stmt, 1);
				results.push_back({id, aura});
			}
		}
		sqlite3_finalize(stmt);
		return results;
	}

	void add_aura(dpp::snowflake guild_id, dpp::snowflake user_id, int amount) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "INSERT INTO aura (guild_id, user_id, amount) VALUES (?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET amount = amount + ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 3, amount);
			sqlite3_bind_int(stmt, 4, amount);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void rmv_aura(dpp::snowflake guild_id, dpp::snowflake user_id, int amount) { add_aura(guild_id, user_id, -amount); }

	int get_total_aura(dpp::snowflake guild_id) {
		std::string g_id_str = std::to_string(guild_id);
		const char* sql = "SELECT SUM(amount) FROM aura WHERE guild_id = ?;";
		sqlite3_stmt* stmt;
		int total = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) { total = sqlite3_column_int(stmt, 0); }
		}
		sqlite3_finalize(stmt);
		return total;
	}

	void set_aura(dpp::snowflake guild_id, dpp::snowflake user_id, int amount) {
		std::string g_id_str = std::to_string(guild_id);
		std::string u_id_str = std::to_string(user_id);
		const char* sql = "INSERT INTO aura (guild_id, user_id, amount) VALUES (?, ?, ?) "
						  "ON CONFLICT(guild_id, user_id) DO UPDATE SET amount = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, u_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 3, amount);
			sqlite3_bind_int(stmt, 4, amount);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	int aura_rank(dpp::snowflake guild_id, dpp::snowflake user_id) {
		// certainly an overstated way to do it but I just learned about coalesce and you know how it is.
		// gotta use it if i learned it lol.
		const char* sql =
			"SELECT count(*) + 1 FROM aura WHERE guild_id = ? "
			"AND amount > (SELECT COALESCE((SELECT amount FROM aura WHERE guild_id = ? AND user_id = ?), 0));";
		sqlite3_stmt* stmt;
		int a_rank = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) a_rank = sqlite3_column_int(stmt, 0);
			sqlite3_finalize(stmt);
		}
		return a_rank;
	}
} // namespace db
