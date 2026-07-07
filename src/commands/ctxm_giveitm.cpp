// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "commands.h"
#include "message.h"
#include "utils.h"

namespace commands {

	dpp::slashcommand giveitm_def(dpp::cluster& bot) {
		return dpp::slashcommand("giveitm", dpp::ctxm_message, bot.me.id).set_default_permissions(dpp::p_administrator);
	}

	void handle_giveitm(const dpp::message_context_menu_t& event, dpp::cluster& bot) {
		dpp::message target = event.get_message();
		if (target.author.is_bot()) {
			event.reply(dpp::message("that's a bot").set_flags(dpp::m_ephemeral));
			return;
		}
		message::do_item_drop(bot, event.command.guild_id, target.author.id, target.channel_id);
		event.reply(dpp::message("rolled.").set_flags(dpp::m_ephemeral));
	}

} // namespace commands
