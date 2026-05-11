#include <algorithm>

#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand auraboard_def(dpp::cluster& bot) {
		return dpp::slashcommand("auraboard", "who has lost the most aura?", bot.me.id)
			.add_option(dpp::command_option(dpp::co_string, "sort", "top or bottom 10", true)
							.add_choice(dpp::command_option_choice("top", std::string("top")))
							.add_choice(dpp::command_option_choice("bottom", std::string("bottom")))
							.add_choice(dpp::command_option_choice("me", std::string("me"))));
	}

	void handle_auraboard(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		std::string sort_mode = std::get<std::string>(event.get_parameter("sort"));
		dpp::snowflake user_id = event.command.get_issuing_user().id;

		if (sort_mode == "me") {
			int aura = db::get_aura(event.command.guild_id, user_id);
			event.reply(dpp::message("You have: " + std::to_string(aura) + " aura").set_flags(dpp::m_ephemeral));
			return;
		}

		auto ab_list = db::get_ab(event.command.guild_id, 15, sort_mode == "bottom");

		std::string embed = sort_mode == "bottom" ? "# AURALESS\n" : "**# AURAFUL\n**";
		int count = 1;

		std::string txt = "";
		for (const auto& row : ab_list) {
			std::string aura_val = std::to_string(row.second) + "AURA";
			txt += "**" + std::to_string(count) + "** <@" + row.first + ">: " + aura_val + "\n";
			count++;
		}

		embed += txt;
		embed += "**total aura:** " + std::to_string(db::get_total_aura(event.command.guild_id));
		event.reply(dpp::message(event.command.channel_id, embed));
	}
} // namespace commands
