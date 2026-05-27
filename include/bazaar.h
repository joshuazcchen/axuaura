#pragma once
#include <dpp/dpp.h>

#include "db.h"

namespace bazaar {
	void b_post_ui(dpp::cluster& bot, dpp::snowflake g_id);
	void b_refresh_guild(dpp::cluster& bot, dpp::snowflake g_id);
	void b_refresh_all(dpp::cluster& bot);
	void b_banner_equip(dpp::snowflake g_id, dpp::snowflake u_id, const db::ShopItem& item);
	void b_banner_unequip(dpp::snowflake g_id, dpp::snowflake u_id);
} // namespace bazaar
