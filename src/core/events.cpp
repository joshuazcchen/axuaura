#include <events.h>
#include "db.h"
#include "xp.h"
#include "config.h"
#include <random>
#include <algorithm>
#include <regex>
#include <cmath>
#include <ctime>

namespace events {
	std::string resp_msg(const std::vector<std::string>& pool) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, pool.size() - 1);
		return pool[dis(gen)];
	}

	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot) {
		if (event.msg.author.is_bot()) return;

		static std::random_device rd;
		static std::mt19937 gen(rd());

		dpp::snowflake user_id = event.msg.author.id;

		long time_now = std::time(nullptr);
		long time_prev = db::xp_time_get(event.msg.guild_id, user_id);

		if (time_now - time_prev >= config::XP_COOLDOWN) {
			std::uniform_int_distribution<> xp_dis(config::XP_MIN, config::XP_MAX);

			int xp_del = xp_dis(gen);
			db::xp_add(event.msg.guild_id, user_id, xp_del);
			db::xp_time_set(event.msg.guild_id, user_id, time_now);
			int xp_now = db::xp_get(event.msg.guild_id, user_id);
			int lvl_now = db::lvl_get(event.msg.guild_id, user_id);
			int xp_requ = xp::xp_req(lvl_now + 1);

			if (xp_now >= xp_requ) {
				int lvl_new = lvl_now + 1;
				db::xp_lvl_set(event.msg.guild_id, user_id, xp_now, lvl_new);
				bot.message_create(dpp::message(config::LVL_CH, "<@" + std::to_string(user_id) + "> reached **level " + std::to_string(lvl_new) + "**, nice."));
				if (config::LVL_ROLES.find(lvl_new) != config::LVL_ROLES.end()) {
					bot.guild_member_add_role(event.msg.guild_id, user_id, config::LVL_ROLES.at(lvl_new));
				}
				// also gonna give them aura for it since I can later lol. TODO: that
			}

		}
		auto& allowed = config::ALLOWED_CHANNELS;
		if (std::find(allowed.begin(), allowed.end(), event.msg.channel_id) == allowed.end()) return;

		std::uniform_int_distribution<> dis(1, 100);
		auto is_special = std::find(config::SPECIALS.begin(), config::SPECIALS.end(), user_id);
		if (is_special != config::SPECIALS.end() && dis(gen) == 1) {
			db::rmv_aura(event.msg.guild_id, user_id, 100);
			bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" HOLY AURA LOSS 💔"));
			return;
		}

		if (dis(gen) <= db::get_setting_int(event.msg.guild_id, "aurachancegain", 10)) {
			db::add_aura(event.msg.guild_id, user_id, db::get_setting_int(event.msg.guild_id, "aurapassiveamt", 2));
		}

		std::regex url_r(R"(https?://[^\s]+)");
		std::smatch match;
		std::string content = event.msg.content;

		if (std::regex_search(content, match, url_r)) {
			std::string url = match.str();
			bool is_reliable = false;
			dpp::channel* ch = dpp::find_channel(event.msg.channel_id);
			if (!ch) return; // i dont know if we'll ever actually need this check but honestly it was there in the original copy so i dont wanna risk it
			auto perm = ch->get_user_permissions(&(event.msg.author));
			for (const auto& p : config::RELIABLE_PROVIDERS) if (url.find(p) != std::string::npos) is_reliable = true;
			if (!is_reliable || content.find("<" + url + ">") != std::string::npos) return;
			for (const auto& attachment: event.msg.attachments) {
				if (attachment.filename.size() >= 4 && (attachment.filename.substr(attachment.filename.size() -4) == ".gif" || attachment.filename.substr(attachment.filename.size() -4) == ".png" || attachment.filename.substr(attachment.filename.size() - 4) == ".jpg")) {
					if (ch) {
						db::rmv_aura(event.msg.guild_id, event.msg.author.id, db::get_setting_int(event.msg.guild_id, "auralossamt", 100));
						bot.channel_typing(event.msg.channel_id);
						std::string msg = (event.msg.channel_id == config::NON_ENG_CH) ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);  
						dpp::message rep(event.msg.channel_id, msg);
						rep.set_reference(event.msg.id);
						bot.message_create(rep);
						return;
					}
				}
			}
			if (ch) {
				dpp::snowflake msg_id = event.msg.id;
				dpp::snowflake ch_id = event.msg.channel_id;
				dpp::snowflake user_sf = event.msg.author.id;
				dpp::snowflake g_id = event.msg.guild_id;
				if (!(perm & dpp::p_embed_links)) {
					bot.channel_typing(event.msg.channel_id);
					bot.start_timer([&bot, msg_id, 	ch_id, user_sf, g_id] (dpp::timer t) {
							db::rmv_aura(g_id, user_sf, db::get_setting_int(g_id, "auralossamt", 100));

							bot.channel_typing(ch_id);
							std::string msg = (ch_id == config::NON_ENG_CH) ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);  
							dpp::message rep(ch_id, msg);
							rep.set_reference(msg_id);
							bot.message_create(rep);
							bot.stop_timer(t);}, 3);
					return;
				} else {
					bot.start_timer([&bot, msg_id, ch_id, url, user_sf, g_id](dpp::timer t) {
							bot.message_get(msg_id, ch_id, [&bot, ch_id, msg_id, url, user_sf, g_id](const dpp::confirmation_callback_t& res) {

									if (res.is_error()) return;
									dpp::message m = std::get<dpp::message>(res.value);
									if (m.embeds.empty() && m.content.find("<" + url + ">") == std::string::npos) {
									db::rmv_aura(g_id, user_sf, db::get_setting_int(g_id, "auralossamt", 100));

									std::string msg = (ch_id == config::NON_ENG_CH) ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);  
									dpp::message rep(ch_id, msg);
									rep.set_reference(msg_id);
									bot.message_create(rep);
									}
									});
							bot.stop_timer(t);}, 5);
				}
			}
		}
	}
}
