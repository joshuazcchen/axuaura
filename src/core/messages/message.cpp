// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "message.h"

#include <ctime>
#include <random>
#include <regex>

#include "config.h"
#include "db.h"
#include "xp.h"

namespace message {

	void handle(const dpp::message_create_t& event, dpp::cluster& bot) {
		if (event.msg.author.is_bot()) return;
		dpp::snowflake g_id = event.msg.guild_id;
		dpp::snowflake u_id = event.msg.author.id;
		auto conf = config::get_config(g_id);

		xp::award(bot, g_id, u_id, event.msg.channel_id, conf.lvl_ch);

		thread_local std::philox4x32 gen(std::random_device{}());
		std::uniform_int_distribution<> dis(1, 100);

		auto& allowed = conf.allowed_channels;
		if (std::find(allowed.begin(), allowed.end(), event.msg.channel_id) == allowed.end()) return;

		// TODO: discuss whether we still even want the special list since neat is no longer spl
		auto is_special = std::find(conf.specials.begin(), conf.specials.end(), u_id);
		if (is_special != conf.specials.end() && dis(gen) == 1) {
			db::rmv_aura(g_id, u_id, 100);
			bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" HOLY AURA LOSS 💔"));
			return;
		}

		if (dis(gen) <= conf.aurachancegain) { db::add_aura(event.msg.guild_id, u_id, conf.aurapassiveamt); }

		std::uniform_int_distribution<> drop_dis(1, 1000);
		if (drop_dis(gen) == 1) do_item_drop(bot, g_id, u_id, event.msg.channel_id);

		std::regex url_r(R"(https?://[^\s]+)");
		std::smatch match;
		std::string content = event.msg.content;
		if (!std::regex_search(content, match, url_r)) return;

		std::string url = match.str();
		bool is_reliable = false;
		dpp::channel* ch = dpp::find_channel(event.msg.channel_id);
		if (!ch) return;
		auto perm = ch->get_user_permissions(&(event.msg.author));
		for (const auto& p : config::RELIABLE_PROVIDERS)
			if (url.find(p) != std::string::npos) is_reliable = true;
		if (!is_reliable || content.find("<" + url + ">") != std::string::npos) return;

		for (const auto& attachment : event.msg.attachments) {
			if (attachment.filename.size() >= 4) {
				auto ext = attachment.filename.substr(attachment.filename.size() - 1);
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if ((ext == ".gif" || ext == ".png" || ext == ".jpg")) {
					bool fail = (attachment.width == 0 || attachment.height == 0);
					if (fail) {
						do_aura_loss(bot, g_id, u_id, event.msg.channel_id, event.msg.id,
									 event.msg.channel_id == conf.non_eng_ch);
						return;
					}
				}
			}
		}

		if (!(perm & dpp::p_embed_links)) {
			bot.channel_typing(event.msg.channel_id);
			dpp::snowflake msg_id = event.msg.id;
			dpp::snowflake ch_id = event.msg.channel_id;
			bool non_eng = (ch_id == conf.non_eng_ch);
			bot.start_timer(
				[&bot, msg_id, ch_id, g_id, u_id, non_eng](dpp::timer t) {
					do_aura_loss(bot, g_id, u_id, ch_id, msg_id, non_eng);
					bot.stop_timer(t);
				},
				3);
		} else {
			dpp::snowflake msg_id = event.msg.id;
			dpp::snowflake ch_id = event.msg.channel_id;
			bool non_eng = (ch_id == conf.non_eng_ch);
			bot.start_timer(
				[&bot, msg_id, ch_id, url, g_id, u_id, non_eng](dpp::timer t) {
					bot.message_get(
						msg_id, ch_id,
						[&bot, ch_id, msg_id, url, g_id, u_id, non_eng](const dpp::confirmation_callback_t& res) {
							if (res.is_error()) return;
							dpp::message m = std::get<dpp::message>(res.value);
							if (m.embeds.empty() && m.content.find("<" + url + ">") == std::string::npos)
								do_aura_loss(bot, g_id, u_id, ch_id, msg_id, non_eng);
						});
					bot.stop_timer(t);
				},
				5);
		}
	}

} // namespace message
