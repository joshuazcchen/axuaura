#include <algorithm>
#include <atomic>
#include <malloc.h>

#include "commands.h"
#include "db.h"
#include "image.h"

namespace commands {

	static std::atomic_bool lb_rendering{false};

	static std::string get_display_name(dpp::snowflake guild_id, dpp::snowflake user_id) {
		dpp::guild* g = dpp::find_guild(guild_id);
		if (g) {
			auto mit = g->members.find(user_id);
			if (mit != g->members.end()) {
				if (!mit->second.get_nickname().empty()) return mit->second.get_nickname();
			}
		}
		dpp::user* u = dpp::find_user(user_id);
		if (u && !u->username.empty()) return u->username;
		return std::to_string(user_id);
	}

	static bool is_guild_member(dpp::snowflake guild_id, dpp::snowflake user_id) {
		dpp::guild* g = dpp::find_guild(guild_id);
		if (!g) return true;
		return g->members.find(user_id) != g->members.end();
	}

	dpp::slashcommand leaderboard_def(dpp::cluster& bot) {
		return dpp::slashcommand("leaderboard", "leaderboard", bot.me.id);
	}

	void handle_leaderboard(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.thinking();
		dpp::snowflake g_id = event.command.guild_id;

		auto raw = db::xp_top(g_id, 50);
		if (raw.empty()) {
			event.edit_original_response(dpp::message("nobody has any XP yet."));
			return;
		}

		std::vector<image::BoardEntry> entries;
		entries.reserve(15);
		for (auto& [uid, xp] : raw) {
			if (!is_guild_member(g_id, uid)) continue;
			entries.push_back({get_display_name(g_id, uid), xp, db::lvl_get(g_id, uid)});
			if ((int)entries.size() >= 15) break;
		}

		if (entries.empty()) {
			event.edit_original_response(dpp::message("leaderboard is empty."));
			return;
		}

		while (lb_rendering.exchange(true)) {}
		std::string img;
		try { img = image::img_gen_leaderboard(entries); } catch (...) {}
		lb_rendering = false;
		malloc_trim(0);

		if (img.empty()) {
			event.edit_original_response(dpp::message("couldn't generate leaderboard image."));
			return;
		}

		dpp::message msg(event.command.channel_id, "");
		msg.add_file("leaderboard.png", img);
		event.edit_original_response(msg);
	}

} // namespace commands
