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

		bot.request(av_url, dpp::m_get, [event, stats, u_id, u_ser](const dpp::http_request_completion_t& result) {
				if (result.status != 200) {
					event.edit_original_response(dpp::message("Something went wrong"));
					return;
				}

				std::string card = image::img_gen_card(
						result.body,
						u_ser.username,
						stats.level,
						stats.xp,
						stats.xp_next,
						stats.progress
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
