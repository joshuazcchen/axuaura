// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#pragma once
#include <dpp/dpp.h>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace config {
	inline const std::vector<std::string> RELIABLE_PROVIDERS = {"tenor.com", "giphy.com",	"youtube.com", "youtu.be",
																"klipy.com", "twitter.com", "tiktok.com"};

	inline const std::vector<std::string> AURA_LOSSES = {
		"**HOLY AURA LOSS <:heartem:1485404606480515172>**",
		"aura loss <:heartem:1485404606480515172>",
		"<:heartem:1485404606480515172> <:skem:1485404723501863085> aura loss <:downvote:1485404530924589228>",
		"Aura loss <:heartem:1485404606480515172>",
		"damn you got less aura than axuaxi after that <:heartem:1485404606480515172>",
		"holy aura loss <:heartem:1485404606480515172>"};

	inline const std::vector<std::string> SPANISH_LOSS = {
		"adios aura <:heartem:1485404606480515172>", "au rev'aura <:heartem:1485404606480515172>", "AURA ⬇️  ",
		"你的aura减少。", "Aura berkurang <:heartem:1485404606480515172>"};

	inline const int BAZAAR_PGSZ = 10;

	struct GuildConfig {
		std::vector<dpp::snowflake> allowed_channels;
		dpp::snowflake non_eng_ch = 0;
		dpp::snowflake log_ch = 0;
		dpp::snowflake lvl_ch = 0;

		dpp::snowflake leader_role = 0;
		dpp::snowflake num2_role = 0;
		dpp::snowflake num3_role = 0;
		dpp::snowflake loser_role = 0;
		dpp::snowflake bot2_role = 0;
		dpp::snowflake bot3_role = 0;

		std::vector<dpp::snowflake> stupid_roles;

		// TODO: make this server specific
		int xp_cooldown = 60;
		int xp_min = 15;
		int xp_max = 30;

		int aurachancegain = 10;
		int aurapassiveamt = 5;

		std::vector<dpp::snowflake> specials;

		void update_stupid() { stupid_roles = {leader_role, num2_role, num3_role, loser_role, bot2_role, bot3_role}; }
	};

	// i dont particularly like this idea but its either i flood my database or i flood my ram with like 1MB of stuff
	extern std::unordered_map<dpp::snowflake, GuildConfig> guild_configs;

	inline GuildConfig get_config(dpp::snowflake guild_id) {
		if (guild_configs.find(guild_id) != guild_configs.end()) return guild_configs[guild_id];
		return GuildConfig();
	}

	void config_load();
} // namespace config
