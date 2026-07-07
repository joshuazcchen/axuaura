// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <random>

#include "commands.h"
#include "db.h"

namespace commands {

	void gamble_do_slots(const dpp::slashcommand_t& event, dpp::cluster& bot, dpp::snowflake user_id, int64_t bet,
						 int aura) {
		event.reply("<a:gamble1:1501132151788142723><a:gamble2:1501132167093289001><a:gamble3:1501132182498840596>");

		new dpp::oneshot_timer(&bot, 3, [event, bet, user_id, aura](dpp::timer) {
			static const std::vector<std::vector<std::string>> ops = {
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
				 "<:row2column1:1501021299726614829><:row2column2:1501021312825430087>"},
			};
			static const std::vector<int> rates = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
												   1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7};

			thread_local std::philox4x32 rng(std::random_device{}());
			std::uniform_int_distribution<size_t> dist(0, rates.size() - 1);
			int i1 = rates[dist(rng)], i2 = rates[dist(rng)], i3 = rates[dist(rng)];

			float mult = 0;
			std::string rslt;

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
				int pair = (i1 == i2) ? i1 : (i2 == i3 ? i2 : i1);
				if (pair >= 5) {
					mult = 5.0f;
					rslt = "DOUBLE RARE 5x";
				} else if (pair >= 2) {
					mult = 2.5f;
					rslt = "DOUBLE UNCOMMON 2.5x";
				} else {
					mult = 1.5f;
					rslt = "DOUBLE COMMON 1.5x";
				}
			} else {
				rslt = (i1 >= 6 || i2 >= 6 || i3 >= 6) ? "no" : "L LOSER 0x";
			}

			if (mult > 0) {
				int del = (int)(bet * mult);
				db::add_aura(event.command.guild_id, user_id, del);
				rslt += "\n" + std::to_string(aura) + "->" + std::to_string((aura - bet) + del) + " (+" +
						std::to_string(del - bet) + ")";
			} else {
				rslt +=
					"\n" + std::to_string(aura) + "->" + std::to_string(aura - bet) + " (-" + std::to_string(bet) + ")";
			}

			event.edit_original_response(dpp::message(ops[i1][0] + ops[i2][0] + ops[i3][0] + "\n" + ops[i1][1] +
													  ops[i2][1] + ops[i3][1] + "\n\n" + rslt));
		});
	}

} // namespace commands
