// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#pragma once
#include <dpp/dpp.h>

namespace bazaar {
	void b_post_ui(dpp::cluster& bot, dpp::snowflake g_id);
	void b_refresh_guild(dpp::cluster& bot, dpp::snowflake g_id, bool force = false);
	void b_refresh_all(dpp::cluster& bot);
} // namespace bazaar
