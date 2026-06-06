// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <algorithm>

#include "commands.h"
#include "config.h"
#include "db.h"

namespace commands {

	dpp::slashcommand fixlevel_def(dpp::cluster& bot) { return dpp::slashcommand("fixlevel", "fix level", bot.me.id); }

	void handle_fixlevel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::guild_member member = event.command.member;
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		dpp::snowflake g_id = event.command.guild_id;
		auto conf = config::get_config(g_id);

		int c_lvl = db::lvl_get(event.command.guild_id, u_id);

		bool changed = false;
		std::vector<dpp::snowflake> c_roles = member.get_roles();

		for (const auto& [lv, rid] : conf.lvl_roles) {
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

		if (changed) {
			event.reply(dpp::message("did something, roles fixed.").set_flags(dpp::m_ephemeral));
		} else {
			event.reply(dpp::message("nothing to change").set_flags(dpp::m_ephemeral));
		}
		return;
	}
} // namespace commands
