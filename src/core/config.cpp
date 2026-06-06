// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "config.h"

#include <sqlite3.h>

#include <iostream>
#include <sstream>

#include "db.h"

namespace config {

	std::unordered_map<dpp::snowflake, GuildConfig> guild_configs;

	static std::vector<dpp::snowflake> parse_sf(const std::string& in) {
		std::vector<dpp::snowflake> result;
		std::stringstream ss(in);
		std::string token;

		while (std::getline(ss, token, ',')) {
			if (!token.empty()) {
				try {
					result.push_back(std::stoull(token));
				} catch (...) {};
			}
		}
		return result;
	}

	void config_load() {
		// maybe i find a way to do this without the sql in the config but honestly its nbd
		const char* sql = "SELECT guild_id, key, value FROM guild_settings;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db::db_ptr, sql, -1, &stmt, nullptr) != SQLITE_OK) { return; }

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			std::string guild_id_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			std::string key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

			dpp::snowflake guild_id = std::stoull(guild_id_str);
			GuildConfig& current_guild = guild_configs[guild_id];

			if (key == "non_eng_ch")
				current_guild.non_eng_ch = std::stoull(value);
			else if (key == "log_ch")
				current_guild.log_ch = std::stoull(value);
			else if (key == "lvl_ch")
				current_guild.lvl_ch = std::stoull(value);
			else if (key == "leader_role")
				current_guild.leader_role = std::stoull(value);
			else if (key == "num2_role")
				current_guild.num2_role = std::stoull(value);
			else if (key == "num3_role")
				current_guild.num3_role = std::stoull(value);
			else if (key == "loser_role")
				current_guild.loser_role = std::stoull(value);
			else if (key == "bot2_role")
				current_guild.bot2_role = std::stoull(value);
			else if (key == "bot3_role")
				current_guild.bot3_role = std::stoull(value);
			else if (key == "specials") {
				current_guild.specials = parse_sf(value);
			} else if (key == "allowed_channels") {
				current_guild.allowed_channels = parse_sf(value);
			} else if (key == "XP_MIN")
				current_guild.xp_min = std::stoi(value);
			else if (key == "XP_MAX")
				current_guild.xp_max = std::stoi(value);
			else if (key == "XP_COOLDOWN")
				current_guild.xp_cooldown = std::stoi(value);
			else if (key == "aurachancegain")
				current_guild.aurachancegain = std::stoi(value);
			else if (key == "aurapassiveamt")
				current_guild.aurapassiveamt = std::stoi(value);
		}
		sqlite3_finalize(stmt);
		for (auto& pair : guild_configs) {
			pair.second.update_stupid();
		}
	}
} // namespace config
