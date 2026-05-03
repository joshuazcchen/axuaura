#include "commands.h"
#include <regex>

namespace commands {
	dpp::slashcommand credits_def(dpp::cluster& bot) {
		dpp::slashcommand cmd("credits", "bot credits", bot.me.id);
		cmd.add_option(dpp::command_option(dpp::co_string, "content", "msg", true));
		return cmd;
	}

	void handle_echo(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.reply(dpp::message("bot by @corgisays\n-# ps: give me money."));
	}

}
