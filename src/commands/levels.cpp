#include <algorithm>
#include <atomic>
#include <malloc.h>

#include "commands.h"
#include "db.h"
#include "events.h"
#include "image.h"
#include "xp.h"

namespace commands {

	// TODO: move these into their own folder

	std::atomic_bool is_rendering{false};

	struct ImgTaskData {
		std::string av_png;
		std::string user;
		int level;
		int xp_now;
		int xp_next;
		float progress;
		std::string bg_c;
		std::string artist;
		bool invert;
		std::string result;
	};

	// TODO: see above, but just tryna pass it thru now for functionality without massively messing up cmake
	void* generate_image_worker(void* arg) {
		ImgTaskData* data = static_cast<ImgTaskData*>(arg);
		data->result = image::img_gen_card(data->av_png, data->user, data->level, data->xp_now, data->xp_next,
				data->progress, data->bg_c, data->artist, data->invert);
		return nullptr;
	}

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
				if (result.status != 200) {
				event.edit_original_response(dpp::message("Something went wrong"));
				return;
				}

				if (result.body.empty()) {
				event.edit_original_response(dpp::message("Something went wrong!"));
				return;
				}

				std::string bg =
				db::get_setting_str(event.command.guild_id, "bg_override_" + std::to_string(u_id), "");
				std::string artist =
				db::get_setting_str(event.command.guild_id, "bg_artist_" + std::to_string(u_id), "");
				bool invert =
				db::get_setting_bool(event.command.guild_id, "bg_invert_" + std::to_string(u_id), 1);
				ImgTaskData task_data{result.body, u_ser.username, stats.level, xp_this, xp_next, p_pct,
				bg,		   artist,		   invert,		""};

				is_rendering = true;
				std::string card;

				{
					try {
						card = image::img_gen_card(task_data.av_png, task_data.user, task_data.level,
								task_data.xp_now, task_data.xp_next, task_data.progress,
								task_data.bg_c, task_data.artist, task_data.invert);
					}
					catch (const std::exception& e) {
						std::cout << "ImageMagick Error: " << e.what() << "\n";
						card = "";
					}
				}
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
