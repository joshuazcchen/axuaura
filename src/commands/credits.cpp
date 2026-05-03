#include "commands.h"
#include <regex>

namespace commands {
	dpp::slashcommand credits_def(dpp::cluster& bot) {
		dpp::slashcommand cmd("credits", "bot credits", bot.me.id);
		return cmd;
	}

	void handle_credits(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.reply(dpp::message("bot by @corgisays\n-# ps: [give me money.](https://ko-fi.com/corig)"));
	}

}
