// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <algorithm>
#include <atomic>
#include <malloc.h>

#include "commands.h"
#include "db.h"
#include "image.h"
#include "utils.h"

namespace commands {

	static std::atomic_bool lb_rendering{false};

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
			if (!utils::is_guild_member(g_id, uid)) continue;
			entries.push_back(
				{utils::get_display_name(g_id, uid), utils::get_avatar_url(uid), xp, db::lvl_get(g_id, uid)});
			if ((int)entries.size() >= 15) break;
		}

		if (entries.empty()) {
			event.edit_original_response(dpp::message("leaderboard is empty."));
			return;
		}

		while (lb_rendering.exchange(true)) {}
		std::string img;
		try {
			img = image::img_gen_leaderboard(entries);
		} catch (...) {}
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
