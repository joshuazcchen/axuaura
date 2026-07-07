// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <algorithm>

#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand smite_def(dpp::cluster& bot) {
		return dpp::slashcommand("smite", "call upon axuaura himself to smite your opponent", bot.me.id)
			.add_option(dpp::command_option(dpp::co_user, "target", "whomst", true))
			.add_option(dpp::command_option(dpp::co_integer, "sacrifice", "your sacrifice to the axuaura", true));
	}

	void handle_smite(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sgn = [](int x) { return x >= 0 ? 1 : -1; };
		dpp::snowflake user_id = event.command.get_issuing_user().id;
		int64_t sacrifice = std::get<int64_t>(event.get_parameter("sacrifice"));
		dpp::snowflake target = std::get<dpp::snowflake>(event.get_parameter("target"));

		if (target == user_id) {
			event.reply(dpp::message("dont waste my time"));
			return;
		}

		if (target == bot.me.id) {
			event.reply(dpp::message("idiot **I AM AURA**"));
			return;
		}

		if (target == 1492320672838582322) {
			event.reply(dpp::message("no. he terrifies even me"));
			return;
		}

		if (target == 1484398643417972766) {
			event.reply(dpp::message("hey man no charge for this one it already has no aura"));
			return;
		}

		if (target == 697952992007028776) {
			event.reply(dpp::message("..."));
			return;
		}

		if (target == 175422893449150464) {
			event.reply(dpp::message("hey man, he lowkey made me so like, yk."));
			return;
		}

		if (sacrifice < 10000 && sacrifice > -10000) {
			event.reply(dpp::message("dont waste my time with that chump change"));
			return;
		}

		int u_aura = db::get_aura(event.command.guild_id, user_id);
		if (std::abs(u_aura) >= std::abs(sacrifice) && sgn(u_aura) == sgn(sacrifice)) {
			event.reply(dpp::message("I see... we have a deal."));
			db::rmv_aura(event.command.guild_id, user_id, sacrifice);
		} else {
			event.reply(dpp::message("show me the money first, then we can talk."));
			return;
		}

		new dpp::oneshot_timer(&bot, 3, [event, target, sacrifice, sgn](dpp::timer t) {
			int t_aura = db::get_aura(event.command.guild_id, target);
			int smite = std::abs((int)(sacrifice / 3));
			if (smite >= std::abs(t_aura)) {
				db::set_aura(event.command.guild_id, target, 0);
				event.edit_original_response(dpp::message("it is done. <@" + std::to_string(target) + "> (" +
														  std::to_string(t_aura) + " -> 0)"));
				return;
			} else {
				db::rmv_aura(event.command.guild_id, target, sgn(t_aura) * smite);
				event.edit_original_response(dpp::message("it is done. <@" + std::to_string(target) + "> (" +
														  std::to_string(t_aura) + " -> " +
														  std::to_string(t_aura - (sgn(t_aura) * smite)) + ")"));
				return;
			}
		});
	}
} // namespace commands
