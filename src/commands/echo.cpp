#include "commands.h"
#include <regex>

namespace commands {
	dpp::slashcommand echo_def(dpp::cluster& bot) {
		dpp::slashcommand cmd("echo", "echo", bot.me.id);
		cmd.set_default_permissions(dpp::p_administrator);
		cmd.set_dm_permission(false);
		cmd.add_option(dpp::command_option(dpp::co_string, "content", "msg", true));
		return cmd;
	}

	void handle_echo(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		//        std::string msg = std::get<std::string>(event.get_parameter("content"));
		//        std::string user_id = std::to_string(event.command.get_issuing_user().id);
		//
		//        if (msg[0] != '!') {
		//            event.reply(dpp::message(event.command.channel_id, msg + "\n-# Sent by <@" + user_id + ">."));
		//        } else {
		//            bot.message_create(dpp::message(event.command.channel_id, std::regex_replace(msg.substr(1), std::regex(R"(\\n)"), "\n")));
		//            event.reply(dpp::message("ok").set_flags(dpp::m_ephemeral));
		//        }
		event.reply(dpp::message("command disabled").set_flags(dpp::m_ephemeral));
	}

}
