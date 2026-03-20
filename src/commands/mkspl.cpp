#include "commands.h"
#include <config.h>

namespace commands {

	void handle_mkspl(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake usr = std::get<dpp::snowflake>(event.get_parameter("user"));
		auto it = std::find(SPECIALS.begin(), SPECIALS.end(), usr);

		if (it != SPECIALS.end()) {
			SPECIALS.erase(it);
			event.reply(dpp::message("user removed").set_flags(dpp::m_ephemeral));
		} else {
			SPECIALS.push_back(usr);
			event.reply(dpp::message("user added").set_flags(dpp::m_ephemeral));
		}
	}
}
