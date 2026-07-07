// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <algorithm>

#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand coinflip_def(dpp::cluster& bot) { return dpp::slashcommand("coinflip", "coinflip", bot.me.id); }

	void handle_coinflip(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		std::vector<std::string> op;
		if (rand() % 2 == 0) {
			op = {"<a:heads4:1501132228946559006>", "<:IMG_1910:1501378267574435981>"};
		} else {
			op = {"<a:tails4:1501132214577139733>", "<:IMG_1912:1501378298893303938>"};
		}

		event.reply(dpp::message(op[0]));
		new dpp::oneshot_timer(&bot, 2,
							   [event, op](dpp::timer t) { event.edit_original_response(dpp::message(op[1])); });
	}
} // namespace commands
