#include "commands.h"
#include "db.h"
#include <algorithm>

namespace commands {

	dpp::slashcommand gamble_def(dpp::cluster& bot) {
		return dpp::slashcommand("gamble", "WHY ISNT THIS WORKING", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "slots", "slots WHY ISNT THIS WORKING")
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
				std::vector<std::vector<std::string>> ops = {
                    {"<:hem1:1485432681176105030><:hem2:1485432706400780510>","<:hem3:1485432730169905312><:hem4:1485432750982037595>"},
                    {"<:row1column1:1485433570175750144><:row1column2:1485433591310581810>","<:row2column1:1485433611724390462><:row2column2:1485433628237496422>"},
                    {"<:row1column1:1485433789365620816><:row1column2:1485434362576109709>","<:row2column1:1485433864754167919><:row2column2:1485433882852462602>"},
                    {"<:row1column1:1489394206253518848><:row1column2:1489394227409326232>","<:row2column1:1489394256110948482><:row2column2:1489394273941061652>"},
                    {"<:row1column1:1489394309198250044><:row1column2:1489394322972348416>","<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"},
                    {"<:row1column1:1489394459748864292><:row1column2:1489394446733807882>","<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"},
                    {"<:row1column1:1489394373480288256><:row1column2:1489394388235718798>","<:row2column1:1489394406132809980><:row2column2:1489394424663511196>"},
                    {"<:row1column1:1489394511007186944><:row1column2:1489394530137407529>","<:row2column1:1489394548143689850><:row2column2:1489394566992892054>"} 
                };

				std::vector<int> rates = {
                    0, 0, 0, 0, 0, 0,
                    1, 1, 1, 1, 1, 1,
                    2, 2,
                    3, 3,
                    4,
                    5,
                    6,
                    7
                };

				int i1 = rates[rand() % rates.size()];
                int i2 = rates[rand() % rates.size()];
                int i3 = rates[rand() % rates.size()];

                std::vector<std::string> r1 = ops[i1];
                std::vector<std::string> r2 = ops[i2];
                std::vector<std::string> r3 = ops[i3];

				int mult = 0;
				std::string rslt = "";

				if (i1 == i2 && i2 == i3) {
                    if (i1 == 7) { mult = 500; rslt = "HOLY AURA GAIN 500x"; }
                    else if (i1 == 6) { mult = 200; rslt = "TRIPLE RARE 200x"; }
                    else if (i1 == 5) { mult = 100; rslt = "TRIPLE RARE 100x"; }
                    else if (i1 >= 2) { mult = 30; rslt = "TRIPLE UNCOMMON 30x"; }
                    else { mult = 3; rslt = "TRIPLE COMMON 3x"; }
                } else if (i1 == i2 || i2 == i3 || i1 == i3) {
                    int pair_id = (i1 == i2) ? i1 : i3;
                    if (pair_id >= 2) { mult = 2; rslt = "DOUBLE UNCOMMON 2x"; }
                    else { mult = 1; rslt = "DOUBLE COMMON 1x"; }
                } else {
                    bool has_single = (i1 >= 5 || i2 >= 5 || i3 >= 5); 
                    
                    if (has_single) {
                        mult = 1;
                        rslt = "SINGLE 1x";
                    } else {
                        mult = 0;
                        rslt = "L LOSER 0x";
                    }
                }

				if (mult > 0) {
					int del = bet * mult;
					db::add_aura(user_id, del);
					long net = (long) del - bet;
					rslt = rslt + "\naura went to " + std::to_string((aura - bet) + del);
				} else { // this is pretty useless of a check but im too lazy to do it properly since i just modified the old aura gambling odds basically lol
					rslt = rslt + "\naura went to " + std::to_string(aura-bet) + "(-" + std::to_string(bet) + ")";
				}
				event.edit_original_response(dpp::message(r1[0] + r2[0] + r3[0] + "\n" + r1[1] + r2[1] + r3[1] + "\n\n" + rslt));
			});
		} else if (cmd.name == "flip") {
			std::vector<std::string> op;
			std::vector<std::string> opp;
			std::string choice = std::get<std::string>(event.get_parameter("choice"));
			if (choice == "heads") {
				op = {"<:row1column1:1489394459748864292><:row1column2:1489394446733807882>","<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"};
				opp = {"<:row1column1:1489394309198250044><:row1column2:1489394322972348416>","<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"};
			} else {
				opp = {"<:row1column1:1489394459748864292><:row1column2:1489394446733807882>","<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"};
				op = {"<:row1column1:1489394309198250044><:row1column2:1489394322972348416>","<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"};
			}
			bool win = (rand() % 2 == 0);

			if (win) {
				int rslt = (int)(bet * 1.5);
				db::add_aura(user_id, rslt);
				event.reply(dpp::message(op[0] + "\n" + op[1] + " \nyour win! aura went to: " + std::to_string(aura + (int)(bet * 0.5))));
			} else {
				event.reply(dpp::message(opp[0] + "\n" + opp[1]  + " \nyour lose! aura went to: " + std::to_string(aura - (int)(bet))));
			}
		}

	}
}
