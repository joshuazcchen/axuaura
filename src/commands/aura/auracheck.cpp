// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <algorithm>

#include "commands.h"
#include "db.h"

namespace commands {

	static void aura_reply(const dpp::slashcommand_t& event, dpp::snowflake g_id, dpp::snowflake target,
						   dpp::snowflake caller) {
		int aura = db::get_aura(g_id, target);
		int rank = db::aura_rank(g_id, target);
		bool self = (target == caller);
		std::string who = self ? "You have" : "<@" + std::to_string(target) + "> has";
		std::string body = who + " **" + std::to_string(aura) + "** aura (rank #" + std::to_string(rank) + ")";
		event.reply(
			dpp::message(body).set_flags(dpp::m_ephemeral).set_allowed_mentions(false, false, false, false, {}, {}));
	}

	dpp::slashcommand aura_def(dpp::cluster& bot) {
		return dpp::slashcommand("aura", "check aura", bot.me.id)
			.add_option(dpp::command_option(dpp::co_user, "who", "check smo else", false));
	}

	void handle_aura(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake caller = event.command.get_issuing_user().id;
		auto who_param = event.get_parameter("who");
		dpp::snowflake target =
			std::holds_alternative<dpp::snowflake>(who_param) ? std::get<dpp::snowflake>(who_param) : caller;
		aura_reply(event, g_id, target, caller);
	}

} // namespace commands
