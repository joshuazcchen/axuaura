#include "commands.h"
#include "db.h"
#include <algorithm>

namespace commands {

	dpp::slashcommand gamble_def(dpp::cluster& bot) {
		return dpp::slashcommand("gamble", "like horse gambling without the payout", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "slots", "slots")
					.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "flip", "lame")
			.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true))
			.add_option(dpp::command_option(dpp::co_string, "choice", "axuaxi or lamexuaxi", true)
			.add_choice(dpp::command_option_choice("heads", std::string("heads")))
			.add_choice(dpp::command_option_choice("tails", std::string("tails")))));
	}

	void handle_gamble(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto cmd = event.command.get_command_interaction().options[0];
		dpp::snowflake user_id = event.command.get_issuing_user().id;
		int64_t bet = std::get<int64_t>(event.get_parameter("bet"));
		int aura = db::get_aura(user_id);
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
		db::rmv_aura(user_id, bet);
		if (cmd.name == "slots") {
			event.reply("<a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957>");
			new dpp::oneshot_timer(&bot, 3, [event, bet, user_id, aura](dpp::timer t) {
				std::vector<std::string> op = {"<:uem:1485404699992785017>","<:skem:1485404723501863085>","<:rem:1485404681105838140>","<:heartem:1485404606480515172>","<:qmem:1485404665331061000>","<:aem:1485404432429613216>","<:duelem:1485404560292974713>","<:downvote:1485404530924589228>"};
				std::string r1 = op[rand() % op.size()]; // im not fucking making 3 random standard unif dists.
				std::string r2 = op[rand() % op.size()];
				std::string r3 = op[rand() % op.size()];

				std::string rslt = "";
				if (r1 == r2 && r2 == r3) {
					db::add_aura(user_id, bet*3);
					rslt = "triple: aura went to: " + std::to_string((aura - bet) + (bet * 3));
				} else if (r1 == r2 || r2 == r3 || r1 == r3) {
					db::add_aura(user_id, bet*2);
					rslt = "double: aura went to: " + std::to_string((aura - bet) + (bet * 2));
				} else {
					rslt = "loser, aura went to " + std::to_string(aura-bet);
				}
				event.edit_original_response(dpp::message(r1 + r2 + r3 + "\n\n"+ rslt));
			});
		} else if (cmd.name == "flip") {
			std::string op = std::get<std::string>(event.get_parameter("choice")) == "heads" ? "<:heads:1485409040485060608>" : "<:skem:1485404723501863085>";
			std::string opp = std::get<std::string>(event.get_parameter("choice")) != "heads" ? "<:heads:1485409040485060608>" : "<:skem:1485404723501863085>";
			bool win = (rand() % 2 == 0);

			if (win) {
				int rslt = (int)(bet * 1.5);
				db::add_aura(user_id, rslt);
				event.reply(dpp::message(op + " \nyour win! aura went to: " + std::to_string(aura + (int)(bet * 0.5))));
			} else {
				event.reply(dpp::message(opp + " \nyour lose! aura went to: " + std::to_string(aura - (int)(bet))));
			}
		}

	}
}
