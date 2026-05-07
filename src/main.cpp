#include <dpp/dpp.h>
#include <Magick++.h>
#include <iostream>
#include <cstdlib>
#include "commands.h"
#include "db.h"
#include "roles.h"
#include "events.h"
#include "dotenv.h"
#include "config.h"

int main(int argc, char **argv) {
	dotenv::init();

	const char *token = std::getenv("BOT_TOKEN");
	setenv("MAGICK_OCL_DEVICE", "OFF", 1); 
	setenv("MAGICK_OPENCL", "OFF", 1);
	Magick::InitializeMagick(*argv);
	Magick::ResourceLimits::thread(1);
	setenv("MAGICK_TEMPORARY_PATH", "/dev/shm", 1);
	Magick::ResourceLimits::memory(4096ull * 1024 * 1024);
	Magick::ResourceLimits::area(4096ull * 1024 * 1024);
	if (!token) {
		exit(1);
	}

	db::init();

	dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_ready([&bot](const dpp::ready_t &event) {
			bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, "ponderin"));	
			commands::register_all(bot);
			config::config_load();
			bot.start_timer([&bot](dpp::timer t) {
				roles::role_sync(bot);
			}, 600);
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
	
	bot.on_voice_state_update([&bot](const dpp::voice_state_update_t& event) {
			events::handle_voice(event, bot);	
			});

	bot.start(dpp::st_wait);
	return 0;
}
