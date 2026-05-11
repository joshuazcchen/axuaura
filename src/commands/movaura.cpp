#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand movaura_def(dpp::cluster& bot) {
		return dpp::slashcommand("movaura", "update aura for user", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.add_option(dpp::command_option(dpp::co_user, "user", "who", true))
			.add_option(dpp::command_option(dpp::co_string, "mode", "mode", true)
							.add_choice(dpp::command_option_choice("set", std::string("set")))
							.add_choice(dpp::command_option_choice("rmv", std::string("rmv")))
							.add_choice(dpp::command_option_choice("add", std::string("add"))))
			.add_option(dpp::command_option(dpp::co_integer, "amt", "amount", true));
	}

	void handle_movaura(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake target_id = std::get<dpp::snowflake>(event.get_parameter("user"));
		std::string mode = std::get<std::string>(event.get_parameter("mode"));
		int64_t amt = std::get<int64_t>(event.get_parameter("amt"));

		std::string response;

		if (mode == "rmv") {
			db::rmv_aura(event.command.guild_id, target_id, amt);
			response = "holy aura loss, " + std::to_string(amt) + " from <@" + target_id.str() + ">.";
		} else if (mode == "add") {
			db::add_aura(event.command.guild_id, target_id, amt);
			response = "holy aura gain, " + std::to_string(amt) + " to <@" + target_id.str() + ">.";
		} else if (mode == "set") {
			db::set_aura(event.command.guild_id, target_id, (int)amt);
			response = "holy aura, " + std::to_string(amt) + " >> <@" + target_id.str() + ">.";
		}

		event.reply(dpp::message(event.command.channel_id, response));
	}
} // namespace commands
