// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <ctime>
#include <sqlite3.h>

#include "db.h"

namespace db {

	void inv_add(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
		sqlite3_stmt* s;
		const char* sql = "INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
						  "VALUES (?, ?, ?, ?, 0, 0)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_bind_int64(s, 4, std::time(nullptr));
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void inv_rm(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
		sqlite3_stmt* s;
		const char* sql = "DELETE FROM inventory WHERE inv_id = ("
						  "SELECT inv_id FROM inventory WHERE guild_id = ? AND user_id = ? AND item_id = ?"
						  "ORDER BY acquired DESC LIMIT 1)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void inv_purge(dpp::snowflake g_id, dpp::snowflake u_id) {
		sqlite3_stmt* s;
		long now = std::time(nullptr);
		const char* sql = "DELETE FROM inventory WHERE guild_id = ? AND user_id = ? "
						  "AND expires > 0 AND expires <= ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(s, 3, now);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void inv_rm_by_inv_id(int inv_id) {
		sqlite3_stmt* s;
		const char* sql = "DELETE FROM inventory WHERE inv_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(s, 1, inv_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	bool inv_has(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
		sqlite3_stmt* s;
		bool has = false;
		const char* sql = "SELECT 1 FROM inventory WHERE guild_id = ? AND user_id = ? AND item_id = ? LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			if (sqlite3_step(s) == SQLITE_ROW) has = true;
			sqlite3_finalize(s);
		}
		return has;
	}

	bool inv_has_xp(dpp::snowflake g_id, dpp::snowflake u_id) {
		sqlite3_stmt* s;
		bool has = false;
		long now = std::time(nullptr);
		const char* sql = "SELECT 1 FROM inventory i JOIN shop_items si ON i.item_id = si.item_id "
						  "WHERE i.guild_id = ? AND i.user_id = ? AND si.type = 'xp_boost' "
						  "AND (i.expires = 0 OR i.expires > ?) LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(s, 3, now);
			if (sqlite3_step(s) == SQLITE_ROW) has = true;
			sqlite3_finalize(s);
		}
		return has;
	}

	std::vector<dpp::snowflake> inv_list(dpp::snowflake g_id, int item_id) {
		sqlite3_stmt* s;
		std::vector<dpp::snowflake> users;
		const char* sql = "SELECT user_id FROM inventory WHERE guild_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, item_id);
			while (sqlite3_step(s) == SQLITE_ROW) {
				const char* r = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
				if (r) users.push_back(std::stoull(r));
			}
			sqlite3_finalize(s);
		}
		return users;
	}

	std::vector<InvItem> inv_get_user(dpp::snowflake g_id, dpp::snowflake u_id) {
		inv_purge(g_id, u_id);
		std::vector<InvItem> items;
		sqlite3_stmt* s;
		const char* sql =
			"SELECT i.inv_id, s.item_id, s.name, s.type, s.role_id, i.equipped, i.acquired, i.expires, s.data"
			" FROM inventory i JOIN shop_items s ON i.item_id = s.item_id"
			" WHERE i.guild_id = ? AND i.user_id = ? ORDER BY i.inv_id";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW) {
				InvItem i;
				i.inv_id = sqlite3_column_int(s, 0);
				i.item_id = sqlite3_column_int(s, 1);
				i.name = reinterpret_cast<const char*>(sqlite3_column_text(s, 2));
				i.type = reinterpret_cast<const char*>(sqlite3_column_text(s, 3));
				i.role_id = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(s, 4)));
				i.equipped = sqlite3_column_int(s, 5) == 1;
				i.acquired = sqlite3_column_int64(s, 6);
				i.expires = sqlite3_column_int64(s, 7);
				if (auto* d = sqlite3_column_text(s, 8)) i.data = reinterpret_cast<const char*>(d);
				items.push_back(i);
			}
			sqlite3_finalize(s);
		}
		return items;
	}

	void inv_eq(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
		sqlite3_stmt* s;
		const char* sql = "UPDATE inventory SET equipped = 1 WHERE guild_id = ? AND user_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void inv_uneq(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
		sqlite3_stmt* s;
		const char* sql = "UPDATE inventory SET equipped = 0 WHERE guild_id = ? AND user_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 3, item_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void inv_uneq_all_type(dpp::snowflake g_id, dpp::snowflake u_id, const std::string& type) {
		sqlite3_stmt* s;
		const char* sql = "UPDATE inventory SET equipped = 0 WHERE guild_id = ? AND user_id = ?"
						  " AND item_id IN (SELECT item_id FROM shop_items WHERE type = ?)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 3, type.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

} // namespace db
