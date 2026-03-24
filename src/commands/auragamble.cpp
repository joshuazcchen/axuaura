#include "commands.h"
#include "db.h"
#include <algorithm>

namespace commands {

	dpp::slashcommand gamble_def(dpp::cluster& bot) {
		return dpp::slashcommand("gamble", "like horse gambling without the payout", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "slots", "slots")
					.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "super_slots", "super_slots")
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
				std::vector<std::vector<std::string>> op = {{"<:hem1:1485432681176105030><:hem2:1485432706400780510>","<:hem3:1485432730169905312><:hem4:1485432750982037595>"},{"<:row1column1:1485433570175750144><:row1column2:1485433591310581810>","<:row2column1:1485433611724390462><:row2column2:1485433628237496422>"},{"<:row1column1:1485433657887035535><:row1column2:1485433672990593044>","<:row2column1:1485433689402904606><:row2column2:1485433705727266936>"},{"<:row1column1:1485433725142437988><:row1column2:1485433740594384926>","<:row2column1:1485433755509461112><:row2column2:1485433769337946152>"},{"<:row1column1:1485433789365620816><:row1column2:1485434362576109709>","<:row2column1:1485433864754167919><:row2column2:1485433882852462602>"}};
				std::vector<std::string> r1 = op[rand() % op.size()];
				std::vector<std::string> r2 = op[rand() % op.size()];
				std::vector<std::string> r3 = op[rand() % op.size()];

				std::string rslt = "";
				if (r1 == r2 && r2 == r3) {
					db::add_aura(user_id, bet*3);
					rslt = "triple: aura went to: " + std::to_string((aura - bet) + (bet * 3)) + "(+" + std::to_string(bet*2) + ")";
				} else if (r1 == r2 || r2 == r3 || r1 == r3) {
					db::add_aura(user_id, bet*2);
					rslt = "double: aura went to: " + std::to_string((aura - bet) + (bet * 2)) + "(+" + std::to_string(bet) + ")";
				} else {
					rslt = "loser, aura went to " + std::to_string(aura-bet) + "(-" + std::to_string(bet) + ")";
				}
				event.edit_original_response(dpp::message(r1[0] + r2[0] + r3[0] + "\n" + r1[1] + r2[1] + r3[1] + "\n\n" + rslt));
			});
		} else if (cmd.name == "super_slots") {
			event.reply("<a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957> <a:slotspin:1485406147430055957>");
			new dpp::oneshot_timer(&bot, 3, [event, bet, user_id, aura](dpp::timer t) {
				std::vector<std::vector<std::string>> op = {{"<:hem1:1485432681176105030><:hem2:1485432706400780510>","<:hem3:1485432730169905312><:hem4:1485432750982037595>"},{"<:row1column1:1485433570175750144><:row1column2:1485433591310581810>","<:row2column1:1485433611724390462><:row2column2:1485433628237496422>"},{"<:row1column1:1485433657887035535><:row1column2:1485433672990593044>","<:row2column1:1485433689402904606><:row2column2:1485433705727266936>"},{"<:row1column1:1485433725142437988><:row1column2:1485433740594384926>","<:row2column1:1485433755509461112><:row2column2:1485433769337946152>"},{"<:row1column1:1485433789365620816><:row1column2:1485434362576109709>","<:row2column1:1485433864754167919><:row2column2:1485433882852462602>"}};
				std::vector<std::string> r1 = op[rand() % op.size()];
				std::vector<std::string> r2 = op[rand() % op.size()];
				std::vector<std::string> r3 = op[rand() % op.size()];
				std::vector<std::string> r4 = op[rand() % op.size()];
				std::vector<std::string> r5 = op[rand() % op.size()];
				std::vector<std::string> r6 = op[rand() % op.size()];
				std::vector<std::string> r7 = op[rand() % op.size()];
				std::vector<std::string> r8 = op[rand() % op.size()];
				std::vector<std::string> r9 = op[rand() % op.size()];

				std::vector<std::vector<std::string>> slots = {r1, r2, r3, r4, r5, r6, r7, r8, r9};
				int match = 0;

				for (auto& s : slots) {
					int count = std::count(slots.begin(), slots.end(), s);
					if (count > match) match = count;
				}

				std::string rslt = "";
				if (match < 4) {
					rslt = std::to_string(match) + " matches. loser. (-" + std::to_string(bet) + ")";
				} else if (match == 9) {
					rslt = "HOLY AURA YOU GOT 9 MATCHES!!! +" + std::to_string(bet * 20);
					db::add_aura(user_id, bet*19);
				} else if (match >= 4) {
					db::add_aura(user_id, ((int)((match-1)))*bet);
					rslt = std::to_string(match) + " matches! aura went to: " + std::to_string((aura - bet) + (bet * ((int)((match-1))))) + " (" + std::to_string(bet * ((int)((match-1)))) + ")";
				}
				event.edit_original_response(dpp::message(r1[0] + r2[0] + r3[0] + r4[0] + r5[0] + r6[0] + r7[0] + r8[0] + r9[0] + "\n" + r1[1] + r2[1] + r3[1] + r4[1] + r5[1] + r6[1] +r7[1]+ r8[1] + r9[1] + "\n\n" + rslt));
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
