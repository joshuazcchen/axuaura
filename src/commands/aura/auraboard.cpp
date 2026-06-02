#include <algorithm>

#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand auraboard_def(dpp::cluster& bot) {
		return dpp::slashcommand("auraboard", "who has lost the most aura?", bot.me.id)
			.add_option(dpp::command_option(dpp::co_string, "sort", "top or bottom 10", false)
					.add_choice(dpp::command_option_choice("top", std::string("top")))
					.add_choice(dpp::command_option_choice("bottom", std::string("bottom"))))
			.add_option(dpp::command_option(dpp::co_user, "who", "check a specific user", false));
	}

	static void aura_reply(const dpp::slashcommand_t& event, dpp::snowflake g_id, dpp::snowflake target, dpp::snowflake caller) {
		int aura = db::get_aura(g_id, target);
		int rank = db::aura_rank(g_id, target);
		bool self = (target == caller);
		std::string who  = self ? "You have" : "<@" + std::to_string(target) + "> has";
		std::string body = who + " **" + std::to_string(aura) + "** aura (rank #" + std::to_string(rank) + ")";
		event.reply(dpp::message(body).set_flags(dpp::m_ephemeral)
				.set_allowed_mentions(false, false, false, false, {}, {}));
	}

	void handle_auraboard(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake caller = event.command.get_issuing_user().id;

		auto who_param = event.get_parameter("who");
		if (std::holds_alternative<dpp::snowflake>(who_param)) {
			aura_reply(event, g_id, std::get<dpp::snowflake>(who_param), caller);
			return;
		}

		auto sort_param = event.get_parameter("sort");
		std::string sort_mode = std::holds_alternative<std::string>(sort_param)
			? std::get<std::string>(sort_param)
			: "me";

		if (sort_mode == "me") {
			aura_reply(event, g_id, caller, caller);
			return;
		}

		auto ab_list = db::get_ab(g_id, 15, sort_mode == "bottom");
		std::string out = (sort_mode == "bottom") ? "# AURALESS\n" : "**# AURAFUL\n**";
		int count = 1;
		for (const auto& row : ab_list)
			out += "**" + std::to_string(count++) + "** <@" + row.first + ">: " + std::to_string(row.second) + " AURA\n";
		out += "**total aura:** " + std::to_string(db::get_total_aura(g_id));
		event.reply(dpp::message(event.command.channel_id, out)
				.set_allowed_mentions(false, false, false, false, {}, {}));
	}

} // namespace commands
