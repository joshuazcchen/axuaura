#include "events.h"
#include "db.h"
#include "config.h"
#include <cmath>
#include <ctime>
#include <random>

namespace events {
	void handle_voice(const dpp::voice_state_update_t& event, dpp::cluster& bot) {
		dpp::snowflake user_id = event.state.user_id;
		dpp::user* u = dpp::find_user(user_id);
		if (u && u->is_bot()) return;
		bool u_left = (event.state.channel_id == 0);

		// prevents people from afking in vc but i should probs exclude an afk channel?
		// nvm we dont even have one
		bool u_afk = (event.state.is_self_deaf() || event.state.is_deaf());

		if (u_left || u_afk) {
			long time_old = db::vc_get(user_id);
			if (time_old > 0) {
				long time_del = std::time(nullptr) - time_old;
				int time_m = time_del / 60;
				if (time_m > 0) {
					static std::random_device rd;
					static std::mt19937 gen(rd());
					std::uniform_int_distribution<> dis(config::XP_MIN, config::XP_MAX);

					int xp_del = 0;
					for (int i = 0; i < time_m; i++) {
						xp_del += dis(gen);
					}
					db::xp_add(user_id, xp_del);
					// TODO: make this handle it but rn itll just work with a message.
				}
				db::vc_clr(user_id);
			}
		} else if (!u_afk) {
			if (db::vc_get(user_id) == 0) {
				db::vc_set(user_id, std::time(nullptr));
			}
		}
	}
}
