// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <sqlite3.h>
#include <vector>

#include "db.h"

namespace db {

	void sb_add_channel(dpp::snowflake guild_id, dpp::snowflake channel_id) {
		sqlite3_stmt* s;
		const char* sql = "INSERT OR IGNORE INTO starboard_channels (guild_id, channel_id) VALUES (?, ?)";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(channel_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void sb_remove_channel(dpp::snowflake guild_id, dpp::snowflake channel_id) {
		sqlite3_stmt* s;
		const char* sql = "DELETE FROM starboard_channels WHERE guild_id = ? AND channel_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(channel_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	bool sb_is_allowed(dpp::snowflake guild_id, dpp::snowflake channel_id) {
		sqlite3_stmt* s;
		bool found = false;
		const char* sql = "SELECT 1 FROM starboard_channels WHERE guild_id = ? AND channel_id = ? LIMIT 1";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(s, 2, std::to_string(channel_id).c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(s) == SQLITE_ROW) found = true;
			sqlite3_finalize(s);
		}
		return found;
	}

	std::vector<dpp::snowflake> sb_get_channels(dpp::snowflake guild_id) {
		std::vector<dpp::snowflake> channels;
		sqlite3_stmt* s;
		const char* sql = "SELECT channel_id FROM starboard_channels WHERE guild_id = ?";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(guild_id).c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(s) == SQLITE_ROW) {
				const char* r = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
				if (r) channels.push_back(std::stoull(r));
			}
			sqlite3_finalize(s);
		}
		return channels;
	}

} // namespace db
