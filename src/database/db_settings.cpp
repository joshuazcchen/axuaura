// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <sqlite3.h>

#include <string>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	void set_setting(dpp::snowflake guild_id, const std::string& key, const std::string& val) {
		std::string g_id_str = std::to_string(guild_id);
		const char* sql = "INSERT INTO guild_settings (guild_id, key, value) VALUES (?, ?, ?) "
						  "ON CONFLICT(guild_id, key) DO UPDATE SET value = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, val.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 4, val.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void set_setting(dpp::snowflake guild_id, const std::string& key, int val) {
		set_setting(guild_id, key, std::to_string(val));
	}

	void set_setting(dpp::snowflake guild_id, const std::string& key, bool val) {
		set_setting(guild_id, key, std::string(val ? "true" : "false"));
	}

	std::string get_setting_str(dpp::snowflake guild_id, const std::string& key, std::string default_val) {
		std::string g_id_str = std::to_string(guild_id);
		const char* sql = "SELECT value FROM guild_settings WHERE guild_id = ? AND key = ?;";
		sqlite3_stmt* stmt;
		std::string result = default_val;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, g_id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* text = sqlite3_column_text(stmt, 0);
				if (text) { result = reinterpret_cast<const char*>(text); }
			}
		}
		sqlite3_finalize(stmt);
		return result;
	}

	int get_setting_int(dpp::snowflake guild_id, const std::string& key, int default_val) {
		std::string val = get_setting_str(guild_id, key, std::to_string(default_val));
		return val.empty() ? default_val : std::stoi(val);
	}

	bool get_setting_bool(dpp::snowflake guild_id, const std::string& key, bool default_val) {
		std::string val = get_setting_str(guild_id, key, "false");
		return (val == "true");
	}

	std::vector<dpp::snowflake> settings_get_guilds_with(const std::string& key) {
		std::vector<dpp::snowflake> guilds;
		const char* sql = "SELECT DISTINCT guild_id FROM guild_settings WHERE key = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* raw = sqlite3_column_text(stmt, 0);
				if (raw) guilds.push_back(std::stoull(reinterpret_cast<const char*>(raw)));
			}
		}
		sqlite3_finalize(stmt);
		return guilds;
	}
} // namespace db
