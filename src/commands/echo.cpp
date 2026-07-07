// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <string>

#include "commands.h"

namespace commands {

	dpp::slashcommand echo_def(dpp::cluster& bot) {
		return dpp::slashcommand("echo", "echo", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.add_option(dpp::command_option(dpp::co_string, "msg", "das message", true));
	}

	void handle_echo(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		std::string msg = std::get<std::string>(event.get_parameter("msg"));
		if (msg[0] != '&') {
			event.reply(dpp::message(msg));
		} else {
			event.reply(dpp::message("ok").set_flags(dpp::m_ephemeral));
			bot.message_create(dpp::message(event.command.channel_id, msg.substr(1)));
		}
	}
} // namespace commands
