// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "voice.h"

#include <cmath>
#include <ctime>
#include <random>

#include "config.h"
#include "db.h"

namespace voice {
	void handle(const dpp::voice_state_update_t& event, dpp::cluster& bot) {
		dpp::snowflake user_id = event.state.user_id;
		dpp::user* u = dpp::find_user(user_id);
		if (u && u->is_bot()) return;
		bool u_left = (event.state.channel_id == 0);

		// prevents people from afking in vc but i should probs exclude an afk channel?
		// nvm we dont even have one
		bool u_afk = (event.state.is_self_deaf() || event.state.is_deaf());

		if (u_left || u_afk) {
			long time_old = db::vc_get(event.state.guild_id, user_id);
			if (time_old > 0) {
				long time_del = std::time(nullptr) - time_old;
				int time_m = time_del / 600;
				if (time_m > 0) {
					static std::philox4x32 gen(std::random_device{}());
					dpp::snowflake g_id = event.state.guild_id;
					auto conf = config::get_config(g_id);
					std::uniform_int_distribution<> dis(conf.xp_min, conf.xp_max);

					int xp_del = 0;
					for (int i = 0; i < time_m; i++) {
						xp_del += dis(gen);
					}
					db::xp_add(event.state.guild_id, user_id, xp_del);
				}
				db::vc_clr(event.state.guild_id, user_id);
			}
		} else if (!u_afk) {
			if (db::vc_get(event.state.guild_id, user_id) == 0) {
				db::vc_set(event.state.guild_id, user_id, std::time(nullptr));
			}
		}
	}
} // namespace voice
