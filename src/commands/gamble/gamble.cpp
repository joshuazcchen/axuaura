// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <random>

#include "commands.h"
#include "db.h"

namespace commands {

	void gamble_do_slots(const dpp::slashcommand_t& event, dpp::cluster& bot, dpp::snowflake user_id, int64_t bet,
						 int aura);
	void gamble_do_flip(const dpp::slashcommand_t& event, dpp::cluster& bot, dpp::snowflake user_id, int64_t bet,
						int aura, const std::string& choice);

	dpp::slashcommand gamble_def(dpp::cluster& bot) {
		return dpp::slashcommand("gamble", "capitalism prime", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "slots", "slots")
							.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "flip", "lame")
							.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true))
							.add_option(dpp::command_option(dpp::co_string, "choice", "axuaxi or lamexuaxi", true)
											.add_choice({"heads", std::string("heads")})
											.add_choice({"tails", std::string("tails")})))
			.add_option(dpp::command_option(dpp::co_sub_command, "help", "im confused"));
	}

	void handle_gamble(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto cmd = event.command.get_command_interaction().options[0];
		if (cmd.name == "help") {
			event.reply(dpp::message(
				"there are things called numbers, we use them to count. you have a thing called aura, this is like "
				"money.\nimagine you have $100, and you go to the casino, and put a $100 bill into a slot machine.\n"
				"while the slot machine is spinning, you currently have $0 in your pocket.\nHowever, once the slot "
				"machine finishes spinning, it decides whether or not you earned money!\n-# earning money is generally "
				"considered good.\nif you win 1.5x, you win back your original amount (1x) and everything else is "
				"winnings."));
			return;
		}

		dpp::snowflake user_id = event.command.get_issuing_user().id;
		int64_t bet = std::get<int64_t>(event.get_parameter("bet"));
		int aura = db::get_aura(event.command.guild_id, user_id);

		if (aura >= 0) {
			if (bet <= 0) {
				event.reply(dpp::message("gambling debt?").set_flags(dpp::m_ephemeral));
				return;
			}
			if (bet > aura) {
				event.reply(dpp::message("brokie").set_flags(dpp::m_ephemeral));
				return;
			}
		} else {
			if (bet >= 0) {
				event.reply(dpp::message("with what aura").set_flags(dpp::m_ephemeral));
				return;
			}
			if (bet < aura) {
				event.reply(dpp::message("youre already auraless enough").set_flags(dpp::m_ephemeral));
				return;
			}
		}
		db::rmv_aura(event.command.guild_id, user_id, bet);

		if (cmd.name == "slots") {
			gamble_do_slots(event, bot, user_id, bet, aura);
		} else if (cmd.name == "flip") {
			gamble_do_flip(event, bot, user_id, bet, aura, std::get<std::string>(event.get_parameter("choice")));
		}
	}

} // namespace commands
