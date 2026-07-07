// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "db.h"
#include <sqlite3.h>

namespace db {

	GlobalBanner gb_b_get(dpp::snowflake user_id) {
		GlobalBanner out;
		const char* sql = "SELECT filename, artist, invert FROM global_banners WHERE user_id = ?";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(s) == SQLITE_ROW) {
				if (auto* f = sqlite3_column_text(s, 0)) out.filename = reinterpret_cast<const char*>(f);
				if (auto* a = sqlite3_column_text(s, 1)) out.artist = reinterpret_cast<const char*>(a);
				out.invert = sqlite3_column_int(s, 2) != 0;
				out.found = true;
			}
			sqlite3_finalize(s);
		}
		return out;
	}

	void gb_b_set(dpp::snowflake user_id, const std::string& filename, const std::string& artist, bool invert) {
		const char* sql = "INSERT INTO global_banners (user_id, filename, artist, invert) VALUES (?, ?, ?, ?)"
						  " ON CONFLICT(user_id) DO UPDATE SET filename=?, artist=?, invert=?";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			int iv = invert ? 1 : 0;
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, filename.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 3, artist.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 4, iv);
			sqlite3_bind_text(s, 5, filename.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 6, artist.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(s, 7, iv);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void gb_b_clear(dpp::snowflake user_id) {
		const char* sql = "DELETE FROM global_banners WHERE user_id = ?";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	InvItem inv_b_equipped(dpp::snowflake g_id, dpp::snowflake u_id) {
		InvItem out;
		const char* sql =
			"SELECT i.inv_id, s.item_id, s.name, s.type, s.role_id, i.equipped, i.acquired, i.expires, s.data"
			" FROM inventory i JOIN shop_items s ON i.item_id = s.item_id"
			" WHERE i.guild_id = ? AND i.user_id = ? AND s.type = 'banner' AND i.equipped = 1"
			" LIMIT 1";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(s) == SQLITE_ROW) {
				out.inv_id = sqlite3_column_int(s, 0);
				out.item_id = sqlite3_column_int(s, 1);
				if (auto* n = sqlite3_column_text(s, 2)) out.name = reinterpret_cast<const char*>(n);
				if (auto* t = sqlite3_column_text(s, 3)) out.type = reinterpret_cast<const char*>(t);
				if (auto* r = sqlite3_column_text(s, 4)) out.role_id = std::stoull(reinterpret_cast<const char*>(r));
				out.equipped = sqlite3_column_int(s, 5) == 1;
				out.acquired = sqlite3_column_int64(s, 6);
				out.expires = sqlite3_column_int64(s, 7);
				if (auto* d = sqlite3_column_text(s, 8)) out.data = reinterpret_cast<const char*>(d);
			}
			sqlite3_finalize(s);
		}
		return out;
	}

} // namespace db
