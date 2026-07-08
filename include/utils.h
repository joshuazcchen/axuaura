// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include <dpp/dpp.h>

#include <string>

namespace utils {
	bool is_admin(const dpp::slashcommand_t& event);
	bool is_guild_member(dpp::snowflake guild_id, dpp::snowflake user_id);

	std::string json_str(const std::string& json, const std::string& key);
	bool json_bool(const std::string& json, const std::string& key, bool default_val = false);
	int json_int(const std::string& json, const std::string& key, int default_val = 0);
	double json_doub(const std::string& json, const std::string& key, double default_val = 0.0);

	std::string get_display_name(dpp::snowflake guild_id, dpp::snowflake user_id);
	std::string get_avatar_url(dpp::snowflake user_id, uint16_t size = 64);
	std::string get_safe_role(const std::string& name, const std::string& type, size_t max_len = 18);
	std::string trunc(const std::string& s, size_t max_chars = 18);
	double get_xpboost(dpp::snowflake guild_id, dpp::snowflake user_id);
} // namespace utils
