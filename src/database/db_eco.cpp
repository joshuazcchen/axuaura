// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <sqlite3.h>

#include <cmath>

#include "db.h"

namespace db {
	bool bazaar_buy(std::string guild_id, std::string user_id, int item_id) {
		dpp::snowflake g_id = std::stoull(guild_id);
		dpp::snowflake u_id = std::stoull(user_id);

		db::ShopItem item = db::shop_get(g_id, item_id);
		int current_aura = db::get_aura(g_id, u_id);

		int user_sign = (current_aura >= 0) ? 1 : -1;
		int item_sign = (item.cost >= 0) ? 1 : -1;

		if (user_sign != item_sign || std::abs(current_aura) < std::abs(item.cost)) { return false; }

		db::rmv_aura(g_id, u_id, item.cost);
		db::inv_add(g_id, u_id, item.item_id);

		return true;
	}

	bool bazaar_sell(std::string guild_id, std::string user_id, int inv_id) {
		dpp::snowflake g_id = std::stoull(guild_id);
		dpp::snowflake u_id = std::stoull(user_id);
		int item_id = inv_id;

		if (!db::inv_has(g_id, u_id, item_id)) { return false; }

		db::ShopItem item = db::shop_get(g_id, item_id);
		int refund = item.cost / 10;

		db::add_aura(g_id, u_id, refund);
		db::inv_rm(g_id, u_id, item_id);

		return true;
	}
} // namespace db
