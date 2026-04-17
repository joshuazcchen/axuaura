#include "commands.h"
#include "db.h"
#include <algorithm>
#include "events.h"

namespace commands {

	dpp::slashcommand level_def(dpp::cluster& bot) {
		return dpp::slashcommand("level", "level", bot.me.id)
			.add_option(dpp::command_option(dpp::co_user, "who", "who see level", false));
	}

	void handle_level(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		auto param = event.get_parameter("who");
		if (std::holds_alternative<dpp::snowflake>(param)) {
			u_id = std::get<dpp::snowflake>(param);
		}
		int xp = db::xp_get(u_id);
		int lvl = db::lvl_get(u_id);
		int nexp = events::xp_req(lvl+1);
		int cexp = events::xp_req(lvl);

		int xpx = xp - cexp;
		int xpy = nexp - cexp;
		double xpd = (xpy > 0) ? (double)xpx / xpy : 0.0;

		int xp_bar = 16;
		int xp_bar_fl = std::max(0, std::min(xp_bar, (int)(xpd * xp_bar)));
		int xp_bar_em = xp_bar - xp_bar_fl;
		std::string bar = ">>";
		for (int i = 0; i < xp_bar_fl; i++) bar += "<:emoji_5:1488244256765120665>";
		for (int i = 0; i < xp_bar_em; i++) bar += "<:axuaxi2:1483872588513148990>";
		bar +="<<";

		std::string msg = "<@" + std::to_string(u_id) + "> lvl " + std::to_string(lvl) + "\n" + std::to_string(xpx) + " " + bar + " " + std::to_string(xpy);
		event.reply(dpp::message(msg).set_allowed_mentions(false, false, false, false, {}, {}));
	}
}
