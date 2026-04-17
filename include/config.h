#pragma once
#include <dpp/dpp.h>
#include <vector>
#include <string>

namespace config {
		extern std::vector<dpp::snowflake> SPECIALS;
		extern std::vector<dpp::snowflake> ALLOWED_CHANNELS;
		extern dpp::snowflake NON_ENG_CH;
		extern dpp::snowflake LOG_CH;

		// TODO: these can be consts idc atp though
		extern dpp::snowflake LEADER;
		extern dpp::snowflake NUM2;
		extern dpp::snowflake NUM3;
		extern dpp::snowflake LOSER; 
		extern dpp::snowflake BOT2;
		extern dpp::snowflake BOT3;

		extern std::vector<dpp::snowflake> STUPID_ROLES;

		extern const std::vector<std::string> AURA_LOSSES;
		extern const std::vector<std::string> SPANISH_LOSS;
	    extern const std::vector<std::string> RELIABLE_PROVIDERS;

		extern const std::map<int, dpp::snowflake> LVL_ROLES;
		extern int XP_COOLDOWN;
		extern int XP_MIN;
		extern int XP_MAX;
		extern dpp::snowflake LVL_CH;
}
