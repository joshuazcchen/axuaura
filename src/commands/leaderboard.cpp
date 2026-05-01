#include "commands.h"
#include "db.h"
#include <algorithm>

namespace commands {

	dpp::slashcommand leaderboard_def(dpp::cluster& bot) {
		return dpp::slashcommand("leaderboard", "leaderboard", bot.me.id);
	}

	void handle_leaderboard(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto top = db::xp_top(event.command.guild_id, 15);
		if (top.empty()) {
			event.reply("smth broke");
			return;
		}
		std::string board = "# Most unemployed:\n";
		int rank = 1;
		for (auto& [uid, xp] : top) {
			int lvl = db::lvl_get(event.command.guild_id, uid);
			std::string rank_str = "**" + std::to_string(rank) + "**";
			rank_str += " <@" + std::to_string(uid) + "> lvl " + std::to_string(lvl) + " (" + std::to_string(xp) + ")\n";
			rank++;
			board += rank_str;
		}
		event.reply(dpp::message(board).set_allowed_mentions(false, false, false, false, {}, {}));
		return;
	}
}
