// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <malloc.h>

#include "commands.h"
#include "db.h"
#include "events.h"
#include "image.h"
#include "utils.h"
#include "xp.h"

namespace commands {

	static std::atomic_bool is_rendering{false};

	dpp::slashcommand level_def(dpp::cluster& bot) {
		return dpp::slashcommand("level", "level", bot.me.id)
			.add_option(dpp::command_option(dpp::co_user, "who", "who see level", false));
	}

	void handle_level(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.thinking();
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		auto param = event.get_parameter("who");
		if (std::holds_alternative<dpp::snowflake>(param)) { u_id = std::get<dpp::snowflake>(param); }

		dpp::user u_ser;
		if (u_id == event.command.get_issuing_user().id) {
			u_ser = event.command.get_issuing_user();
		} else {
			u_ser = event.command.get_resolved_user(u_id);
		}

		xp::UserXP stats = xp::xp_getxp(event.command.guild_id, u_id);
		std::string av_url = u_ser.get_avatar_url(256, dpp::i_png, false);
		int cexp = xp::xp_req(stats.level);
		int nexp = xp::xp_req(stats.level + 1);

		int xp_this = stats.xp - cexp;
		int xp_next = nexp - cexp;
		float p_pct = (xp_next > 0) ? (float)xp_this / xp_next : 0.0f;

		bot.request(av_url, dpp::m_get,
					[event, stats, u_id, u_ser, p_pct, xp_this, xp_next](const dpp::http_request_completion_t& result) {
						if (result.status != 200 || result.body.empty()) {
							event.edit_original_response(dpp::message("something went wrong fetching avatar"));
							return;
						}

						std::string bg_path;
						std::string artist;
						bool invert = false;

						db::GlobalBanner gb = db::gb_b_get(u_id);
						if (gb.found) {
							bg_path = "assets/bg/custom/" + gb.filename;
							artist = gb.artist;
							invert = gb.invert;
						} else {
							db::InvItem equipped = db::inv_b_equipped(event.command.guild_id, u_id);
							if (!equipped.data.empty()) {
								std::string file = utils::json_str(equipped.data, "file");
								if (!file.empty()) {
									bg_path = "assets/bg/bazaar/" + file;
									artist = utils::json_str(equipped.data, "artist");
									invert = utils::json_bool(equipped.data, "invert");
								}
							}
						}
						std::vector<std::string> badges = db::badge_get(u_id);

						is_rendering = true;
						std::string card;
						try {
							card = image::img_gen_card(result.body, u_ser.username, stats.level, xp_this, xp_next,
													   p_pct, bg_path, artist, invert, badges);
						} catch (const std::exception& e) { std::cerr << e.what() << "\n"; }
						malloc_trim(0);
						is_rendering = false;

						if (card.empty()) {
							event.edit_original_response(dpp::message("something went wrong generating preview"));
							return;
						}

						dpp::message msg(event.command.channel_id, "");
						msg.add_file("level.png", card);
						event.edit_original_response(msg);
					});
	}
} // namespace commands
