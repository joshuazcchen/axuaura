#include <sqlite3.h>

#include <string>
#include <vector>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	std::vector<Poll> p_get_polls(dpp::snowflake guild_id) {
		std::string g_id_str = std::to_string(guild_id);
		std::vector<Poll> list;
		const char* sql = "SELECT p_id, title, ops FROM polls WHERE guild_id = ? AND active = 1;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				Poll p;
				p.p_id = sqlite3_column_int(stmt, 0);
				p.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				p.ops = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				list.push_back(p);
			}
		}
		sqlite3_finalize(stmt);
		return list;
	}

	Poll p_get_poll(int p_id) { // Guild ID not strictly needed for fetch-by-ID if ID is global AI/PK
		const char* sql = "SELECT p_id, title, ops FROM polls WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		Poll p = {-1, "", ""};
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				p.p_id = sqlite3_column_int(stmt, 0);
				const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				const char* opts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				p.title = title ? title : "";
				p.ops = opts ? opts : "";
			}
		}
		sqlite3_finalize(stmt);
		return p;
	}

	int p_set_poll(dpp::snowflake guild_id, std::string title, std::string ops) {
		std::string g_id_str = std::to_string(guild_id);
		const char* sql = "INSERT INTO polls (guild_id, title, ops) VALUES (?, ?, ?);";
		sqlite3_stmt* stmt;
		int id = -1;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, ops.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
			id = (int)sqlite3_last_insert_rowid(db_ptr);
		}
		sqlite3_finalize(stmt);
		return id;
	}

	std::string p_get_poll_ops(int p_id) {
		const char* sql = "SELECT ops FROM polls WHERE p_id = ? AND active = 1;";
		sqlite3_stmt* stmt;
		std::string ops = "";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* text = sqlite3_column_text(stmt, 0);
				if (text) { ops = reinterpret_cast<const char*>(text); }
			}
		}
		sqlite3_finalize(stmt);
		return ops;
	}

	bool p_get_poll_end(int p_id) {
		const char* sql = "SELECT active FROM polls WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		bool active = false;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) { active = (sqlite3_column_int(stmt, 0) == 1); }
		}
		sqlite3_finalize(stmt);
		return active;
	}

	void p_end_poll(int p_id) {
		const char* sql = "UPDATE polls SET active = 0 WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void p_place_bet(int p_id, dpp::snowflake user_id, std::string op, int amt) {
		std::string s_uid = std::to_string(user_id);
		const char* sql = "INSERT INTO bets (p_id, u_id, op, amt) VALUES (?, ?, ?, ?);";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_bind_text(stmt, 2, s_uid.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, op.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 4, amt);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	long p_get_all_pot(int p_id) {
		const char* sql = "SELECT SUM(amt) FROM bets WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		long total = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) { total = sqlite3_column_int(stmt, 0); }
		}
		sqlite3_finalize(stmt);
		return total;
	}

	long p_get_op_pot(int p_id, std::string op) {
		const char* sql = "SELECT SUM(amt) FROM bets WHERE p_id = ? AND op = ?;";
		sqlite3_stmt* stmt;
		long total = 0;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_bind_text(stmt, 2, op.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) { total = sqlite3_column_int(stmt, 0); }
		}
		sqlite3_finalize(stmt);
		return total;
	}

	std::vector<std::pair<std::string, int>> p_get_op_users(int p_id, std::string op) {
		std::vector<std::pair<std::string, int>> winners;
		const char* sql = "SELECT u_id, amt FROM bets WHERE p_id = ? AND op = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_bind_text(stmt, 2, op.c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				const char* u_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
				int amt = sqlite3_column_int(stmt, 1);
				winners.push_back({u_id, amt});
			}
		}
		sqlite3_finalize(stmt);
		return winners;
	}
} // namespace db
