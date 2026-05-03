#include "commands.h"
#include <regex>

namespace commands {
	dpp::slashcommand credits_def(dpp::cluster& bot) {
		dpp::slashcommand cmd("credits", "bot credits", bot.me.id);
		return cmd;
	}

	void handle_credits(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.reply(dpp::message("bot by [@corgisays](<https://scallion.uk>)\n-# ps: [give me money.](<https://ko-fi.com/corig>)\n\nart by [@notaxuaxi](<https://www.instagram.com/notaxuaxi/>)\n-# subscribe to his [patreon](<https://www.patreon.com/cw/notaxuaxi>).\n\nfor all bot related inquiries, email `corgi@scallion.uk`."));
	}

}
