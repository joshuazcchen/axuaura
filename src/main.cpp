#include <dpp/dpp.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include "commands.h"
#include "db.h"
#include "events.h"
#include "buttons.h"

int main() {
	const char *token = std::getenv("BOT_TOKEN");
	if (!token) {
		exit(1);
	}

	db::init();

	dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content);
	bot.on_ready([&bot](const dpp::ready_t &event) {
			bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, "ponderin"));	
			commands::register_all(bot);
			});

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
			commands::route_slash_command(bot, event);
			});

	bot.on_message_context_menu([&bot](const dpp::message_context_menu_t& event) {
			commands::route_context_menu(bot, event);
			});

	bot.on_message_create([&bot](const dpp::message_create_t& event) {
			if (event.msg.author.is_bot()) return;
			events::handle_message(event, bot);
			});

	bot.on_button_click([&bot](const dpp::button_click_t& event) {
		buttons::handle_button_click(event, bot);
	});

	bot.start(dpp::st_wait);
	return 0;
}
