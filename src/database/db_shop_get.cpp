#include <sqlite3.h>

#include "db.h"

namespace db {

	static ShopItem row_to_shopitem(sqlite3_stmt* s) {
		ShopItem i;
		i.item_id = sqlite3_column_int(s, 0);
		i.type = reinterpret_cast<const char*>(sqlite3_column_text(s, 1));
		i.role_id = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
		i.name = reinterpret_cast<const char*>(sqlite3_column_text(s, 3));
		if (auto* d = sqlite3_column_text(s, 4)) i.desc = reinterpret_cast<const char*>(d);
		i.cost = sqlite3_column_int(s, 5);
		if (auto* d = sqlite3_column_text(s, 6)) i.data = reinterpret_cast<const char*>(d);
		i.active = sqlite3_column_int(s, 7) == 1;
		i.pinned = sqlite3_column_int(s, 8) == 1;
		i.global = sqlite3_column_int(s, 9) == 1;
		return i;
	}

	ShopItem shop_get(dpp::snowflake g_id, int item_id) {
		ShopItem i;
		sqlite3_stmt* s;
		const char* sql = "SELECT item_id, type, role_id, name, desc, cost, data, active, pinned, global"
						  " FROM shop_items WHERE guild_id = ? AND item_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, item_id);
			if (sqlite3_step(s) == SQLITE_ROW) i = row_to_shopitem(s);
			sqlite3_finalize(s);
		}
		return i;
	}

	std::vector<ShopItem> shop_get_all(dpp::snowflake g_id, bool active_only) {
		std::vector<ShopItem> items;
		sqlite3_stmt* s;
		std::string sql = "SELECT item_id, type, role_id, name, desc, cost, data, active, pinned, global"
						  " FROM shop_items WHERE guild_id = ?";
		if (active_only) sql += " AND active = 1";
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW)
				items.push_back(row_to_shopitem(s));
			sqlite3_finalize(s);
		}
		return items;
	}

	std::vector<ShopItem> shop_get_sign(dpp::snowflake g_id, int sign) {
		std::vector<ShopItem> items;
		sqlite3_stmt* s;
		const char* sql = "SELECT item_id, type, role_id, name, desc, cost, data, active, pinned, global"
						  " FROM shop_items WHERE guild_id = ? AND active = 1 AND obtainable = 1"
						  " AND ((? >= 0 AND cost >= 0) OR (? < 0 AND cost < 0))";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 2, sign);
			sqlite3_bind_int(s, 3, sign);
			while (sqlite3_step(s) == SQLITE_ROW)
				items.push_back(row_to_shopitem(s));
			sqlite3_finalize(s);
		}
		return items;
	}

	std::vector<ShopItem> shop_get_pinned(dpp::snowflake g_id) {
		std::vector<ShopItem> items;
		sqlite3_stmt* s;
		const char* sql = "SELECT item_id, type, role_id, name, desc, cost, data, active, pinned, global"
						  " FROM shop_items WHERE guild_id = ? AND pinned = 1 AND active = 1";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW)
				items.push_back(row_to_shopitem(s));
			sqlite3_finalize(s);
		}
		return items;
	}

	std::vector<ShopItem> shop_get_rotating_pool(dpp::snowflake g_id) {
		std::vector<ShopItem> items;
		sqlite3_stmt* s;
		const char* sql = "SELECT item_id, type, role_id, name, desc, cost, data, active, pinned, global"
						  " FROM shop_items WHERE guild_id = ? AND active = 1 AND obtainable = 1 AND pinned = 0";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW)
				items.push_back(row_to_shopitem(s));
			sqlite3_finalize(s);
		}
		return items;
	}

} // namespace db
