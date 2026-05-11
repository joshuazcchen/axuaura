#include <sqlite3.h>

#include <iostream>

#include "config.h"
#include "db.h"

namespace db {
	int shop_add(dpp::snowflake g_id, const std::string& type, dpp::snowflake r_id, const std::string& name,
				 const std::string& desc, int cost, const std::string& data) {
		sqlite3_stmt* s;
		std::string sql =
			"INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data) VALUES (?, ?, ?, ?, ?, ?, ?)";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
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
		std::string sql = "UPDATE shop_items SET active = 0 WHERE guild_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, item_id);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	ShopItem shop_get(dpp::snowflake g_id, int item_id) {
		ShopItem i = {-1, 0, "", "", "", 0, "", false};
		sqlite3_stmt* s;
		std::string sql = "SELECT item_id, type, role_id, name, desc, cost, data, active FROM shop_items WHERE "
						  "guild_id = ? AND item_id "
						  "= ?";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, item_id);
			if (sqlite3_step(s) == SQLITE_ROW) {
				i.item_id = sqlite3_column_int(s, 0);
				i.type = reinterpret_cast<const char*>(sqlite3_column_text(s, 1));
				i.role_id = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
				i.name = reinterpret_cast<const char*>(sqlite3_column_text(s, 3));
				const unsigned char* desc = sqlite3_column_text(s, 4);
				if (desc) i.desc = reinterpret_cast<const char*>(desc);
				i.cost = sqlite3_column_int(s, 5);
				const unsigned char* data = sqlite3_column_text(s, 6);
				if (data) i.data = reinterpret_cast<const char*>(data);
				i.active = sqlite3_column_int(s, 7) == 1;
			}
			sqlite3_finalize(s);
		}
		return i;
	}

	std::vector<ShopItem> shop_get_all(dpp::snowflake g_id, bool active) {
		std::vector<ShopItem> items;
		sqlite3_stmt* s;
		std::string sql =
			std::string("SELECT item_id, type, role_id, name, desc, cost FROM shop_items WHERE guild_id = ? ") +=
			active ? "AND active = 1;" : ";";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW) {
				ShopItem i;
				i.item_id = sqlite3_column_int(s, 0);
				i.type = reinterpret_cast<const char*>(sqlite3_column_text(s, 1));
				i.role_id = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
				i.name = reinterpret_cast<const char*>(sqlite3_column_text(s, 3));
				const unsigned char* desc = sqlite3_column_text(s, 4);
				if (desc) i.desc = reinterpret_cast<const char*>(desc);
				i.cost = sqlite3_column_int(s, 5);
				items.push_back(i);
			}
			sqlite3_finalize(s);
		}
		return items;
	}

	std::vector<ShopItem> shop_get_sign(dpp::snowflake g_id, int sign) {
		std::vector<ShopItem> items;
		sqlite3_stmt* s;
		std::string sql = "SELECT item_id, role_id, name, cost FROM shop_items WHERE guild_id = ? AND active = 1 AND "
						  "((? >= 0 AND cost "
						  ">= 0) OR (? < 0 AND cost < 0)) AND obtainable = 1";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, sign);
			sqlite3_bind_int(s, 3, sign);
			while (sqlite3_step(s) == SQLITE_ROW) {
				ShopItem i;
				i.item_id = sqlite3_column_int(s, 0);
				i.role_id = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(s, 1)));
				i.name = reinterpret_cast<const char*>(sqlite3_column_text(s, 2));
				i.cost = sqlite3_column_int(s, 3);
				items.push_back(i);
			}
			sqlite3_finalize(s);
		}
		return items;
	}

	int shop_state(dpp::snowflake g_id, int item_id, const std::string& key) {
		sqlite3_stmt* s;
		std::string chk = "SELECT " + key + " FROM shop_items WHERE guild_id = ? AND item_id = ? LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, chk.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(item_id).c_str(), -1, SQLITE_TRANSIENT);

			if (sqlite3_step(s) == SQLITE_ROW) {
				int id = sqlite3_column_int(s, 0);
				sqlite3_finalize(s);
				return id;
			}
			sqlite3_finalize(s);
		}
		return -1;
	}

	void shop_set_int(dpp::snowflake g_id, int item_id, const std::string& key, int value) {
		sqlite3_stmt* s;
		std::string sql = "UPDATE shop_items SET " + key + " = ? WHERE guild_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(value).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 3, std::to_string(item_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	int shop_ensure_sys(dpp::snowflake g_id, const std::string& name, dpp::snowflake r_id, int cost) {
		sqlite3_stmt* s;
		std::string chk = "SELECT item_id FROM shop_items WHERE guild_id = ? AND role_id = ? LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, chk.c_str(), -1, &s, nullptr) == SQLITE_OK) {
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
