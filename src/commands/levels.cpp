#include "commands.h"
#include "db.h"
#include "xp.h"
#include "image.h"
#include <algorithm>
#include "events.h"

namespace commands {

	dpp::slashcommand level_def(dpp::cluster& bot) {
		return dpp::slashcommand("level", "level", bot.me.id)
			.add_option(dpp::command_option(dpp::co_user, "who", "who see level", false));
	}

	void handle_level(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.thinking();
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		auto param = event.get_parameter("who");
		if (std::holds_alternative<dpp::snowflake>(param)) {
			u_id = std::get<dpp::snowflake>(param);
		}

		dpp::user u_ser = *dpp::find_user(u_id);

		xp::UserXP stats = xp::xp_getxp(event.command.guild_id, u_id);
		std::string av_url = u_ser.get_avatar_url(256, dpp::i_png, false);
		int cexp = xp::xp_req(stats.level);
		int nexp = xp::xp_req(stats.level + 1);

		int xp_this = stats.xp - cexp;
		int xp_next  = nexp - cexp;
		float p_pct = (xp_next > 0) ? (float) xp_this / xp_next : 0.0f;

		bot.request(av_url, dpp::m_get, [event, stats, u_id, u_ser, p_pct, xp_this, xp_next](const dpp::http_request_completion_t& result) {
				if (result.status != 200) {
					event.edit_original_response(dpp::message("Something went wrong"));
					return;
				}

				std::string bg = db::get_setting_str(event.command.guild_id, "bg_override_" + std::to_string(u_id), "");
				std::string artist = db::get_setting_str(event.command.guild_id, "bg_artist_" + std::to_string(u_id), "");
				bool invert = db::get_setting_bool(event.command.guild_id, "bg_invert_" + std::to_string(u_id), 1);
				std::string card = image::img_gen_card(
						result.body,
						u_ser.username,
						stats.level,
						xp_this,
						xp_next,
						p_pct,
						bg,
						artist,
						invert
					);

				if (card.empty()) {
					event.edit_original_response(dpp::message("something went wrong generating preview"));
					return;
				}

				dpp::message msg(event.command.channel_id, "");
				msg.add_file("level.png", card);
				event.edit_original_response(msg);
		});
	}
}
