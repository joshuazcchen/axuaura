// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "commands.h"
#include "config.h"
#include "db.h"
#include "message.h"
#include "utils.h"

namespace commands {

	dpp::slashcommand mkshm_def(dpp::cluster& bot) {
		return dpp::slashcommand("mkshm", dpp::ctxm_message, bot.me.id).set_default_permissions(dpp::p_administrator);
	}

	void handle_mkshm(const dpp::message_context_menu_t& event, dpp::cluster& bot) {
		dpp::message target = event.get_message();
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = target.author.id;
		const auto& conf = config::get_config(g_id);
		bool non_eng = (target.channel_id == conf.non_eng_ch);

		db::rmv_aura(g_id, u_id, db::get_setting_int(g_id, "auralossamt", 100));

		std::string text = non_eng ? message::resp_msg(config::SPANISH_LOSS) : message::resp_msg(config::AURA_LOSSES);
		dpp::message rep(target.channel_id, text);
		rep.set_reference(target.id);
		bot.message_create(rep);

		event.reply(dpp::message("done.").set_flags(dpp::m_ephemeral));
	}

} // namespace commands
