#include <sqlite3.h>

#include "db.h"

namespace db {

	int shop_add(dpp::snowflake g_id, const std::string& type, dpp::snowflake r_id, const std::string& name,
				 const std::string& desc, int cost, const std::string& data) {
		sqlite3_stmt* s;
		const char* sql = "INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data)"
						  " VALUES (?, ?, ?, ?, ?, ?, ?)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, type.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 3, std::to_string(r_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 4, name.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 5, desc.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 6, cost);
			sqlite3_bind_text(s, 7, data.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			int id = sqlite3_last_insert_rowid(db_ptr);
			sqlite3_finalize(s);
			return id;
		}
		return -1;
	}

	void shop_rmv(dpp::snowflake g_id, int item_id) {
		sqlite3_stmt* s;
		const char* sql = "UPDATE shop_items SET active = 0 WHERE guild_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, item_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	int shop_state(dpp::snowflake g_id, int item_id, const std::string& key) {
		sqlite3_stmt* s;
		std::string sql = "SELECT " + key + " FROM shop_items WHERE guild_id = ? AND item_id = ? LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, item_id);
			if (sqlite3_step(s) == SQLITE_ROW) {
				int val = sqlite3_column_int(s, 0);
				sqlite3_finalize(s);
				return val;
			}
			sqlite3_finalize(s);
		}
		return -1;
	}

	void shop_set_int(dpp::snowflake g_id, int item_id, const std::string& key, int value) {
		sqlite3_stmt* s;
		std::string sql = "UPDATE shop_items SET " + key + " = ? WHERE guild_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(s, 1, value);
			sqlite3_bind_text(s, 2, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	int shop_ensure_sys(dpp::snowflake g_id, const std::string& name, dpp::snowflake r_id, int cost) {
		sqlite3_stmt* s;
		const char* chk = "SELECT item_id FROM shop_items WHERE guild_id = ? AND role_id = ? LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, chk, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(r_id).c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(s) == SQLITE_ROW) {
				int id = sqlite3_column_int(s, 0);
				sqlite3_finalize(s);
				return id;
			}
			sqlite3_finalize(s);
		}
		return shop_add(g_id, "role", r_id, name, "System Managed", cost, "{}");
	}

} // namespace db
