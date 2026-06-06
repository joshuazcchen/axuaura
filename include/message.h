// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#pragma once
#include <dpp/dpp.h>

namespace message {
	void handle(const dpp::message_create_t& event, dpp::cluster& bot);
	std::string resp_msg(const std::vector<std::string>& pool);
	void do_item_drop(dpp::cluster& bot, dpp::snowflake g_id, dpp::snowflake user_id, dpp::snowflake ch_id);
	void do_aura_loss(dpp::cluster& bot, dpp::snowflake g_id, dpp::snowflake user_id, dpp::snowflake ch_id,
					  dpp::snowflake msg_id, bool is_non_eng);
} // namespace message
