// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <algorithm>
#include <nlohmann/json.hpp>

#include "commands.h"
#include "config.h"
#include "db.h"

namespace commands {

	dpp::slashcommand fixlevel_def(dpp::cluster& bot) { return dpp::slashcommand("fixlevel", "fix level", bot.me.id); }

	void handle_fixlevel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::guild_member member = event.command.member;
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		dpp::snowflake g_id = event.command.guild_id;

		int c_lvl = db::lvl_get(event.command.guild_id, u_id);

		bool changed = false;
		std::vector<dpp::snowflake> c_roles = member.get_roles();

		std::string roles_json_str = db::get_setting_str(g_id, "xp_level_roles", "{}");
		try {
			nlohmann::json roles_map = nlohmann::json::parse(roles_json_str);
			for (auto& [lv_str, role_val] : roles_map.items()) {
				int lv = std::stoi(lv_str);
				dpp::snowflake rid = std::stoull(role_val.get<std::string>());
				bool has_role = std::find(c_roles.begin(), c_roles.end(), rid) != c_roles.end();
				if (c_lvl >= lv) {
					if (!has_role) {
						bot.guild_member_add_role(g_id, u_id, rid);
						changed = true;
					}
				} else {
					if (has_role) {
						bot.guild_member_remove_role(g_id, u_id, rid);
						changed = true;
					}
				}
			}
		} catch (...) {}

		if (changed) {
			event.reply(dpp::message("did something, roles fixed.").set_flags(dpp::m_ephemeral));
		} else {
			event.reply(dpp::message("nothing to change").set_flags(dpp::m_ephemeral));
		}
		return;
	}
} // namespace commands
