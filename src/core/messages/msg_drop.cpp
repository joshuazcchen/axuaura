// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "message.h"

#include <cmath>
#include <random>

#include "db.h"

namespace message {

	void do_item_drop(dpp::cluster& bot, dpp::snowflake g_id, dpp::snowflake user_id, dpp::snowflake ch_id) {
		int u_aura = db::get_aura(g_id, user_id);
		int sign = (u_aura >= 0) ? 1 : -1;
		auto items = db::shop_get_sign(g_id, sign);
		if (items.empty()) return;

		auto weight_get = [](int cost) -> double {
			double cost_scaled = std::abs(cost) / 1000.0;
			return 1.0 / std::pow(cost_scaled + 1.0, 2.0);
		};

		double total_weight = 0;
		for (auto& item : items)
			total_weight += weight_get(item.cost);

		thread_local std::philox4x32 rng(std::random_device{}());
		std::uniform_real_distribution<> wdis(0, total_weight);
		double roll = wdis(rng);

		db::ShopItem* picked = &items.front();

		for (auto& item : items) {
			roll -= weight_get(item.cost);
			if (roll <= 0) {
				picked = &item;
				break;
			}
		}

		if (db::inv_has(g_id, user_id, picked->item_id)) {
			int refund = picked->cost / 10;
			db::add_aura(g_id, user_id, refund);
			bot.message_create(dpp::message(ch_id, "<@" + std::to_string(user_id) + "> found **" + picked->name +
													   "** but they already owned it so its been converted to " +
													   std::to_string(refund) + "** aura instead"));
		} else {
			db::inv_add(g_id, user_id, picked->item_id);
			double weight = weight_get(picked->cost);
			double pool = total_weight / weight;
			long long odds = static_cast<long long>(pool * 1000.0);

			bot.message_create(dpp::message(
				ch_id, "<@" + std::to_string(user_id) + "> is lucky as hell and stumbled across a **" + picked->name +
						   "** (1/" + std::to_string(odds) + ") ! access with `/inventory view`."));
		}
	}
} // namespace message
