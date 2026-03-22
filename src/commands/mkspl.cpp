#include "commands.h"
#include <config.h>

namespace commands {
	dpp::slashcommand mkspl_def(dpp::cluster& bot) {
		return dpp::slashcommand("mkspl", "make someone special", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.set_dm_permission(false)
			.add_option(dpp::command_option(dpp::co_user, "user", "who to toggle?", true));
	}

	void handle_mkspl(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake usr = std::get<dpp::snowflake>(event.get_parameter("user"));
		auto it = std::find(config::SPECIALS.begin(), config::SPECIALS.end(), usr);

		if (it != config::SPECIALS.end()) {
			config::SPECIALS.erase(it);
			event.reply(dpp::message("user removed").set_flags(dpp::m_ephemeral));
		} else {
			config::SPECIALS.push_back(usr);
			event.reply(dpp::message("user added").set_flags(dpp::m_ephemeral));
		}
	}
}
