#include <Magick++.h>
#include <dpp/dpp.h>

#include <cstdlib>
#include <iostream>

#include "commands.h"
#include "config.h"
#include "db.h"
#include "dotenv.h"
#include "events.h"
#include "roles.h"

int main(int argc, char** argv) {
	dotenv::init();

	const char* token = std::getenv("BOT_TOKEN");
	setenv("MAGICK_OCL_DEVICE", "ON", 1);
	setenv("MAGICK_OPENCL", "ON", 1);
	setenv("MAGICK_DEBUG", "Accelerate", 1);

	Magick::InitializeMagick(*argv);

	if (!token) { exit(1); }

	db::init();

	dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_ready([&bot](const dpp::ready_t& event) {
		bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, "ponderin"));
		commands::register_all(bot);
		config::config_load();
		bot.start_timer([&bot](dpp::timer t) { roles::role_sync(bot); }, 600);
	});

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) { commands::route_slash_command(bot, event); });

	bot.on_message_context_menu(
		[&bot](const dpp::message_context_menu_t& event) { commands::route_context_menu(bot, event); });

	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		if (event.msg.author.is_bot()) return;
		events::handle_message(event, bot);
	});

	bot.on_voice_state_update([&bot](const dpp::voice_state_update_t& event) { events::handle_voice(event, bot); });

	bot.start(dpp::st_wait);
	return 0;
}
