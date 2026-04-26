#include <events.h>
#include "db.h"
#include "config.h"
#include <random>
#include <algorithm>
#include <regex>
#include <cmath>
#include <ctime>

namespace events {
//	void msg_scan(dpp::cluster& bot, dpp::snowflake target_channel, dpp::snowflake before_id, long cutoff_date, std::shared_ptr<std::map<dpp::snowflake, int>> missed_counts, int scanned_so_far, dpp::snowflake report_channel) {
//
//		bot.messages_get(target_channel, 0, before_id, 0, 100, [&bot, target_channel, cutoff_date, missed_counts, scanned_so_far, report_channel](const dpp::confirmation_callback_t& callback) {
//
//				if (callback.is_error()) {
//					int comped = 0;
//					for (auto& [uid, msg_count] : *missed_counts) {
//						db::xp_add(uid, msg_count * 20);
//						db::xp_migrate_set(uid, true);
//						comped++;
//					}
//					bot.message_create(dpp::message(report_channel, "rate limited at " + std::to_string(scanned_so_far) + " messages. partial done " + std::to_string(comped)));
//					return;
//				}
//
//				auto messages = std::get<dpp::message_map>(callback.value);
//
//				dpp::snowflake oldest_id = (uint64_t)-1;
//				bool hit_cutoff = false;
//
//				for (auto& [msg_id, msg] : messages) {
//					if (msg.id < oldest_id) oldest_id = msg.id;
//
//					if (msg.sent < cutoff_date) {
//						hit_cutoff = true;
//					} else if (!msg.author.is_bot() && !db::xp_migrate_get(msg.author.id)) {
//						(*missed_counts)[msg.author.id]++;
//					}
//				}
//
//				int current_total = scanned_so_far + messages.size();
//
//				if (!hit_cutoff && messages.size() >= 99) { 
//					msg_scan(bot, target_channel, oldest_id, cutoff_date, missed_counts, current_total, report_channel);
//				} else {
//					int comped = 0;
//					for (auto& [uid, msg_count] : *missed_counts) {
//						db::xp_add(uid, msg_count * 20); 
//						db::xp_migrate_set(uid, true);
//						comped++;
//					}
//					bot.message_create(dpp::message(report_channel, "done " + std::to_string(current_total) + "** done. done**" + std::to_string(comped) + "** done."));
//				}
//		});
//	}


	std::string resp_msg(const std::vector<std::string>& pool) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, pool.size() - 1);
		return pool[dis(gen)];
	}

	// TODO: make xp its own separate folder and make it properly separated but that requires moving aura out of being hard coded in here basically.
	//
	int xp_req(int lvl) {
		int xp = 0;

		double xp_base = 75.0;
		double xp_mult = 1.10;
		double xp_cap = 7500.0;

		for (int i = 1; i < lvl; ++i) {
			double xp_del = xp_base * std::pow(xp_mult, i - 1);
			xp += (int)std::min(xp_del, xp_cap);
		}

		return xp;
	}

	// TODO: make the voice chat actually announce a level up. rn it just stores until they send a message.
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot) {
		if (event.msg.author.is_bot()) return;

//		// IM NOT USING A FULL PROPER THING FOR THIS SINCE ITS RUN ONCE TO JUST MIGRATE.
//		if (event.msg.author.id == 175422893449150464ULL) {
//			// lowkey aura farm when all this is just a random message check but YK WHAT I GET TO MIGRATE MESSAGES AND LOOK COOL WHILE DOING IT LOL
//			if (event.msg.content == "SQLEVAL<<.read+FUCKYOU+json") {
//				std::ifstream file("oldxp.json");
//				if (!file.is_open()) {
//					std::cout<<"nope";
//					return;
//				}
//				// im pretty sure dpp includes nlohmann tbh i dont know if i ever needed to use nlohmann LMFAO
//				dpp::json j;
//				file >> j;
//				int count = 0;
//				for (auto& user : j["levels"]) {
//					dpp::snowflake uid = std::stoull(user["userId"].get<std::string>());
//					int xp = user["xp"].get<int>();
//					int lvl = user["level"].get<int>();
//					db::xp_lvl_set(uid, xp, lvl);
//					count++;
//				}
//				// if this isnt 100 im gonna lose it.
//				bot.message_create(dpp::message(event.msg.channel_id, "nah." + std::to_string(count)));
//				return;
//			}
//
//			if (event.msg.content == "SQLEVAL<<.read+FUCKYOU+roles") {
//				dpp::snowflake guild = 1469591363770122274ULL;
//				dpp::snowflake lv5role_thing_idk_this_is_temporary = 1482905611535519877ULL;
//
//				bot.guild_get_members(guild, 1000, 0, [&bot, event, lv5role_thing_idk_this_is_temporary](const dpp::confirmation_callback_t& callback) {
//						if (callback.is_error()) return;
//						auto members = std::get<dpp::guild_member_map>(callback.value);
//						int count = 0;
//						for (auto& [uid, member] : members) {
//							if (db::xp_get(uid) > 0) continue;
//							int lv5 = 0;
//							for (auto& role_id : member.get_roles()) {
//								if (role_id == lv5role_thing_idk_this_is_temporary) {
//									lv5 = 1;
//								}
//							}
//						if (lv5 == 1) {
//							count++;
//							db::xp_lvl_set(uid, 650, 5);
//							}
//						}
//						bot.message_create(dpp::message(event.msg.channel_id, "nah." + std::to_string(count)).set_allowed_mentions(true, false, false, false, {}, {}));
//						});
//				return;
//			}
//
//			if (event.msg.content == "SQLEVAL<<.read+FUCKYOU+msgs") {
//				bot.message_create(dpp::message(event.msg.channel_id, "start"));
//				dpp::snowflake target_channel = event.msg.channel_id;
//				long cutoff_date = 1776225600;
//				auto missed_counts = std::make_shared<std::map<dpp::snowflake, int>>();
//
//                msg_scan(bot, target_channel, 0, cutoff_date, missed_counts, 0, event.msg.channel_id);
//                return;
//			}
//		}
//
//
		// EVERTYTHING ABOVE THIS IS TEMPORARY





		static std::random_device rd;
		static std::mt19937 gen(rd());

		dpp::snowflake user_id = event.msg.author.id;

		long time_now = std::time(nullptr);
		long time_prev = db::xp_time_get(user_id);

		if (time_now - time_prev >= config::XP_COOLDOWN) {
			std::uniform_int_distribution<> xp_dis(config::XP_MIN, config::XP_MAX);

			int xp_del = xp_dis(gen);
			db::xp_add(user_id, xp_del);
			db::xp_time_set(user_id, time_now);
			int xp_now = db::xp_get(user_id);
			int lvl_now = db::lvl_get(user_id);
			int xp_requ = xp_req(lvl_now + 1);

			if (xp_now >= xp_requ) {
				int lvl_new = lvl_now + 1;
				db::xp_lvl_set(user_id, xp_now, lvl_new);
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
			db::rmv_aura(user_id, 100);
			bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" HOLY AURA LOSS 💔"));
			return;
		}

		if (dis(gen) <= db::get_setting_int("aurachancegain", 10)) {
			db::add_aura(user_id, db::get_setting_int("aurapassiveamt", 2));
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
						db::rmv_aura(event.msg.author.id, db::get_setting_int("auralossamt", 100));
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
				if (!(perm & dpp::p_embed_links)) {
					bot.channel_typing(event.msg.channel_id);
					bot.start_timer([&bot, msg_id, 	ch_id, user_sf] (dpp::timer t) {
							db::rmv_aura(user_sf, db::get_setting_int("auralossamt", 100));

							bot.channel_typing(ch_id);
							std::string msg = (ch_id == config::NON_ENG_CH) ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);  
							dpp::message rep(ch_id, msg);
							rep.set_reference(msg_id);
							bot.message_create(rep);
							bot.stop_timer(t);}, 3);
					return;
				} else {
					bot.start_timer([&bot, msg_id, ch_id, url, user_sf](dpp::timer t) {
							bot.message_get(msg_id, ch_id, [&bot, ch_id, msg_id, url, user_sf](const dpp::confirmation_callback_t& res) {

									if (res.is_error()) return;
									dpp::message m = std::get<dpp::message>(res.value);
									if (m.embeds.empty() && m.content.find("<" + url + ">") == std::string::npos) {
									db::rmv_aura(user_sf, db::get_setting_int("auralossamt", 100));

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
