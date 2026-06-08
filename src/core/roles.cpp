// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "roles.h"

#include <dpp/dpp.h>

#include "config.h"
#include "db.h"

namespace roles {
	void role_sync(dpp::cluster& bot) {
		for (auto& [g_id, conf] : config::guild_configs) {
			if (conf.leader_role == 0) continue;

			auto top3 = db::get_ab(g_id, 3, false);
			auto bot3 = db::get_ab(g_id, 3, true);

			int leader_id = db::shop_ensure_sys(g_id, "most aura", conf.leader_role, 50000);
			int loser_id = db::shop_ensure_sys(g_id, "least aura", conf.loser_role, -50000);
			int bot3_id = db::shop_ensure_sys(g_id, "3nd least", conf.bot3_role, -50000);
			int bot2_id = db::shop_ensure_sys(g_id, "2nd least", conf.bot2_role, -50000);
			int num2_id = db::shop_ensure_sys(g_id, "2nd most", conf.num2_role, 50000);
			int num3_id = db::shop_ensure_sys(g_id, "3nd most", conf.num3_role, 50000);

			std::vector<std::pair<dpp::snowflake, int>> r_i_pairs = {
				{conf.leader_role, leader_id}, {conf.num2_role, num2_id}, {conf.num3_role, num3_id},
				{conf.loser_role, loser_id},   {conf.bot2_role, bot2_id}, {conf.bot3_role, bot3_id}};

			std::map<dpp::snowflake, int> targets;
			auto do_rank = [&](const std::vector<std::pair<std::string, int>>& list, const std::vector<int>& ids) {
				for (size_t i = 0; i < list.size() && i < ids.size(); ++i) {
					targets[std::stoull(list[i].first)] = ids[i];
				}
			};

			do_rank(top3, {leader_id, num2_id, num3_id});
			do_rank(bot3, {loser_id, bot2_id, bot3_id});

			for (auto const& [r_id, i_id] : r_i_pairs) {
				auto cur = db::inv_list(g_id, i_id);
				for (auto const& u_id : cur) {
					if (targets.count(u_id) && targets.at(u_id) == i_id) continue;
					bot.guild_member_remove_role(g_id, u_id, r_id);
					db::inv_rm(g_id, u_id, i_id);
				}
				for (auto const& [u_id, a_i_id] : targets) {
					if (a_i_id == i_id) {
						if (!db::inv_has(g_id, u_id, i_id)) {
							db::inv_add(g_id, u_id, i_id);
							db::inv_eq(g_id, u_id, i_id);
							bot.guild_member_add_role(g_id, u_id, r_id);
						}
						break;
					}
				}
			}
		}
	}
} // namespace roles
