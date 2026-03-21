#include "commands.h"
#include "db.h"

namespace commands {
	
	dpp::slashcommand get_settings_definition(dpp::cluster& bot) {
		return dpp::slashcommand("settings", "set ings", bot.me.id) {
			.set_default_permissions(dpp::p_administrator)
			.add_option(
				dpp::command_option(dpp::co_string, "setting", "which one", true)
				.add_choice(dpp::command_option_choice("aura_rate", std::string("aura_rate")))
				.add_choice(dpp::command_option_choice("status", std::string("status")))
			)
			.add_option(dpp::command_option(dpp::co_string, "value", "val"));
		}
	}

	void handle_settings(const dpp::slash_command_t& event, dpp::cluster& bot) {
		std::string target_setting = std::get<std::string>(event.get_parameter("setting"));
		std::string new_val = std::get<std::string>(event.get_parameter("value"));
		
		if (target_setting == "aura_rate") {
			try {
				int rate = std::stoi(new_val);
				db::set_setting("aura_rate", rate);
			} catch (...) {
				event.reply(dpp::message("kys").set_flags(dpp::m_ephemeral));
				return;
			}
		} else if (target_setting == "status") {
			db::set_setting("status", new_val);
			bot.set_presence(dpp::presence(ps_online, dpp::at_game, new_val));
		}

		event.reply(dpp::message("your did it!").set_flags(dpp::m_ephemeral));
	}

}
