// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "commands.h"
#include "db.h"
#include "xp.h"

namespace commands {

	dpp::slashcommand setlevel_def(dpp::cluster& bot) {
		return dpp::slashcommand("setlevel", "set a user's level and/or xp", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.add_option(dpp::command_option(dpp::co_user, "user", "target", true))
			.add_option(dpp::command_option(dpp::co_integer, "level", "level to set", false))
			.add_option(dpp::command_option(dpp::co_integer, "xp", "raw xp override (optional)", false));
	}

	void handle_setlevel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake uid = std::get<dpp::snowflake>(event.get_parameter("user"));

		auto lvl_param = event.get_parameter("level");
		auto xp_param = event.get_parameter("xp");

		bool has_lvl = std::holds_alternative<int64_t>(lvl_param);
		bool has_xp = std::holds_alternative<int64_t>(xp_param);

		if (!has_lvl && !has_xp) {
			event.reply(dpp::message("provide level and/or xp").set_flags(dpp::m_ephemeral));
			return;
		}

		int new_level = has_lvl ? (int)std::get<int64_t>(lvl_param) : db::lvl_get(g_id, uid);
		int new_xp = has_xp ? (int)std::get<int64_t>(xp_param) : xp::xp_req(new_level);

		if (new_level < 0) {
			event.reply(dpp::message("level can't be negative").set_flags(dpp::m_ephemeral));
			return;
		}

		db::xp_set(g_id, uid, new_xp, new_level);
		event.reply(dpp::message("set <@" + std::to_string(uid) + "> -> level **" + std::to_string(new_level) + "** (" +
								 std::to_string(new_xp) + " xp)")
						.set_flags(dpp::m_ephemeral)
						.set_allowed_mentions(false, false, false, false, {}, {}));
	}

} // namespace commands
