#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand settings_def(dpp::cluster& bot) {
		return dpp::slashcommand("settings", "set ings", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.add_option(
					dpp::command_option(dpp::co_string, "setting", "which one", true)
					.add_choice(dpp::command_option_choice("aurachancegain", std::string("aurachancegain")))
					.add_choice(dpp::command_option_choice("aurapassiveamt", std::string("aurapassiveamt")))
					.add_choice(dpp::command_option_choice("auralbozoamt", std::string("auralbozoamt")))
					.add_choice(dpp::command_option_choice("auralossamt", std::string("auralbozoamt")))
					.add_choice(dpp::command_option_choice("duelminwager", std::string("duelminwager")))
					.add_choice(dpp::command_option_choice("status", std::string("status")))
					.add_choice(dpp::command_option_choice("XP_MIN", std::string("XP_MIN")))
					.add_choice(dpp::command_option_choice("XP_MAX", std::string("XP_MAX")))
					.add_choice(dpp::command_option_choice("XP_COOLDOWN", std::string("XP_COOLDOWN")))
				   )
			.add_option(dpp::command_option(dpp::co_string, "value", "val"));
	}

	void handle_settings(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		std::string target_setting = std::get<std::string>(event.get_parameter("setting"));
		std::string new_val = std::get<std::string>(event.get_parameter("value"));

		if (target_setting == "status") {
			db::set_setting(event.command.guild_id, "status", new_val);
			bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, new_val));
		} else {
			try {
				int rate = std::stoi(new_val);
				db::set_setting(event.command.guild_id, target_setting, rate);
			} catch (...) {
				event.reply(dpp::message("kys").set_flags(dpp::m_ephemeral));
				return;
			}
		}

		event.reply(dpp::message("your did it!").set_flags(dpp::m_ephemeral));
	}

}
