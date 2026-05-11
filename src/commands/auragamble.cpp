#include <algorithm>

#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand gamble_def(dpp::cluster& bot) {
		return dpp::slashcommand("gamble", "WHY ISNT THIS WORKING", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "slots", "slots WHY ISNT THIS WORKING")
							.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "flip", "lame")
							.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true))
							.add_option(dpp::command_option(dpp::co_string, "choice", "axuaxi or lamexuaxi", true)
											.add_choice(dpp::command_option_choice("heads", std::string("heads")))
											.add_choice(dpp::command_option_choice("tails", std::string("tails")))))
			.add_option(dpp::command_option(dpp::co_sub_command, "help", "im confused"));
	}

	void handle_gamble(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto cmd = event.command.get_command_interaction().options[0];
		if (cmd.name == "help") {
			event.reply(dpp::message(
				"there are things called numbers, we use them to count. you have a thing called aura, this is like "
				"money.\nimagine you have $100, and you go to the casino, and put a $100 bill into a slot "
				"machine.\nwhile "
				"the slot machine is spinning, you currently have $0 in your pocket.\nHowever, once the slot machine "
				"finishes spinning, it decides whether or not you earned money!\n-# earning money is generally "
				"considered "
				"good.\nthe machines on this bot refer to the amount you win (the number of money that you get back "
				"from "
				"the machine) using a thing called multiplication.\nthis might seem like a big word, but it's actually "
				"really simple!\nokay i cant keep up this act. basically just if you win 1.5x, you win back your "
				"original "
				"amount (1x) and everything else is considered winnings.\nif you have any further questions spam ping "
				"axuaxi not me - corgi\n\nTODO: make this display rates"));
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
			event.reply(
				"<a:gamble1:1501132151788142723><a:gamble2:1501132167093289001><a:gamble3:1501132182498840596>");
			new dpp::oneshot_timer(&bot, 3, [event, bet, user_id, aura](dpp::timer t) {
				std::vector<std::vector<std::string>> ops = {
					{"<:hem1:1485432681176105030><:hem2:1485432706400780510>",
					 "<:hem3:1485432730169905312><:hem4:1485432750982037595>"},
					{"<:row1column1:1485433570175750144><:row1column2:1485433591310581810>",
					 "<:row2column1:1485433611724390462><:row2column2:1485433628237496422>"},
					{"<:row1column1:1485433789365620816><:row1column2:1485434362576109709>",
					 "<:row2column1:1485433864754167919><:row2column2:1485433882852462602>"},
					{"<:row1column1:1489394206253518848><:row1column2:1489394227409326232>",
					 "<:row2column1:1489394256110948482><:row2column2:1489394273941061652>"},
					{"<:row1column1:1489394309198250044><:row1column2:1489394322972348416>",
					 "<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"},
					{"<:row1column1:1489394459748864292><:row1column2:1489394446733807882>",
					 "<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"},
					{"<:row1column1:1489394373480288256><:row1column2:1489394388235718798>",
					 "<:row2column1:1489394406132809980><:row2column2:1489394424663511196>"},
					{"<:row1column1:1501021269401796699><:row1column2:1501021285046550739>",
					 "<:row2column1:1501021299726614829><:row2column2:1501021312825430087>"}};

				std::vector<int> rates = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
										  1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7};

				int i1 = rates[rand() % rates.size()];
				int i2 = rates[rand() % rates.size()];
				int i3 = rates[rand() % rates.size()];

				std::vector<std::string> r1 = ops[i1];
				std::vector<std::string> r2 = ops[i2];
				std::vector<std::string> r3 = ops[i3];

				float mult = 0;
				std::string rslt = "";

				if (i1 == i2 && i2 == i3) {
					if (i1 == 7) {
						mult = 500.0f;
						rslt = "# HOLY AURA 500x";
					} else if (i1 == 6) {
						mult = 200.0f;
						rslt = "TRIPLE RARE 200x";
					} else if (i1 == 5) {
						mult = 75.0f;
						rslt = "TRIPLE RARE 75x";
					} else if (i1 >= 2) {
						mult = 20.0f;
						rslt = "TRIPLE UNCOMMON 20x";
					} else {
						mult = 3.0f;
						rslt = "TRIPLE COMMON 3x";
					}
				} else if (i1 == i2 || i2 == i3 || i1 == i3) {
					int pair_id = (i1 == i2) ? i1 : (i2 == i3 ? i2 : i1);
					if (pair_id >= 5) {
						mult = 5.0f;
						rslt = "DOUBLE RARE 5x";
					} else if (pair_id >= 2) {
						mult = 2.5f;
						rslt = "DOUBLE UNCOMMON 2.5x";
					} else {
						mult = 1.5f;
						rslt = "DOUBLE COMMON 1.5x";
					}
				} else {
					bool has_single = (i1 >= 6 || i2 >= 6 || i3 >= 6);
					if (has_single) {
						mult = 0;
						rslt = "no";
					} else {
						mult = 0;
						rslt = "L LOSER 0x";
					}
				}

				if (mult > 0) {
					if (user_id == 175422893449150464ULL || user_id == 1194435328312881242ULL ||
						user_id == 318540048779968513ULL) {
						mult *= 1.5f;
					}
					int del = bet * mult;
					db::add_aura(event.command.guild_id, user_id, del);
					long net = (long)del - bet;
					rslt = rslt + "\n" + std::to_string(aura) + "->" + std::to_string((aura - bet) + del) + " (" +
						   std::to_string(del - bet) + ")";
					if (user_id == 175422893449150464ULL || user_id == 1194435328312881242ULL ||
						user_id == 318540048779968513ULL || user_id == 603870585482772491ULL) {
						rslt += " (but you're better than everyone else so you get 1.5x that for no reason) ";
					}
				} else { // this is pretty useless of a check but im too lazy to do it properly since i just modified
						 // the old aura gambling odds basically lol
					rslt = rslt + "\n" + std::to_string(aura) + "->" + std::to_string(aura - bet) + " (-" +
						   std::to_string(bet) + ")";
				}
				event.edit_original_response(
					dpp::message(r1[0] + r2[0] + r3[0] + "\n" + r1[1] + r2[1] + r3[1] + "\n\n" + rslt));
			});
		} else if (cmd.name == "flip") {
			std::vector<std::string> op;
			std::vector<std::string> opp;
			std::string choice = std::get<std::string>(event.get_parameter("choice"));
			if (choice == "heads") {
				op = {"<a:heads4:1501132228946559006>",
					  "<:row1column1:1489394459748864292><:row1column2:1489394446733807882>",
					  "<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"};
				opp = {"<a:tails4:1501132214577139733>",
					   "<:row1column1:1489394309198250044><:row1column2:1489394322972348416>",
					   "<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"};
			} else {
				opp = {"<a:heads4:1501132228946559006>",
					   "<:row1column1:1489394459748864292><:row1column2:1489394446733807882>",
					   "<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"};
				op = {"<a:tails4:1501132214577139733>",
					  "<:row1column1:1489394309198250044><:row1column2:1489394322972348416>",
					  "<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"};
			}
			bool win = (rand() % 2 == 0);

			if (win) {
				int rslt = (int)(bet * 1.5);
				db::add_aura(event.command.guild_id, user_id, rslt);
				event.reply(dpp::message(op[0]));
				new dpp::oneshot_timer(&bot, 2, [event, bet, user_id, aura, op](dpp::timer t) {
					event.edit_original_response(
						dpp::message(op[1] + "\n" + op[2] + " \nyour win!\n" + std::to_string(aura) + "->" +
									 std::to_string(aura + (int)(bet * 0.5)) + " (+" + std::to_string(bet / 2) + ")"));
				});
			} else {
				event.reply(dpp::message(opp[0]));
				new dpp::oneshot_timer(&bot, 2, [event, bet, user_id, aura, opp](dpp::timer t) {
					event.edit_original_response(
						dpp::message(opp[1] + "\n" + opp[2] + " \nyour lose!\n" + std::to_string(aura) + "->" +
									 std::to_string(aura - (int)(bet)) + " (-" + std::to_string(bet) + ")"));
				});
			}
		}
	}
} // namespace commands
