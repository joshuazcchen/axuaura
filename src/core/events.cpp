#include <events.h>
#include "db.h"
#include "config.h"
#include <random>
#include <algorithm>
#include <regex>

namespace events {
	std::string resp_msg(const std::vector<std::string>& pool) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, pool.size() - 1);
		return pool[dis(gen)];
	}

	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot) {
		if (event.msg.author.is_bot()) return;

		auto& allowed = config::ALLOWED_CHANNELS;
		if (std::find(allowed.begin(), allowed.end(), event.msg.channel_id) == allowed.end()) return;

		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(1, 100);

		dpp::snowflake user_id = event.msg.author.id;

		auto is_special = std::find(config::SPECIALS.begin(), config::SPECIALS.end(), user_id);
		if (is_special != config::SPECIALS.end() && dis(gen) == 1) {
			db::rmv_aura(user_id, 100);
			bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" HOLY AURA LOSS 💔"));
			return;
		}

		std::cout<<db::get_setting_int("aurachancegain", 10)<<std::endl;
		if (dis(gen) <= db::get_setting_int("aurachancegain", 10)) {
			std::cout<<"passively gained aura!"<<std::endl;
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
			auto perms = ch->get_user_permissions(&(event.msg.author));
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
				if (!(perms & dpp::p_embed_links)) {
					bot.channel_typing(event.msg.channel_id);
					bot.start_timer([&bot, msg_id, 	ch_id, user_sf] (dpp::timer t) {
						db::rmv_aura(user_sf, db::get_setting_int("auralossamt", 100));
							
						bot.channel_typing(ch_id);
						std::string msg = (ch_id == config::NON_ENG_CH) ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);  
						dpp::message rep(ch_id, msg);
						rep.set_reference(msg_id);
						bot.message_create(rep);
					bot.stop_timer(t);}, 5);
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
					bot.stop_timer(t);}, 2);
				}
			}
			}
	}
}
