#include <Magick++.h>
#include <dpp/dpp.h>

#include <cstdlib>
#include <iostream>

#include "backup.h"
#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "dotenv.h"
#include "events.h"
#include "roles.h"

int main(int argc, char** argv) {
	dotenv::init();

	const char* token = std::getenv("BOT_TOKEN");

	Magick::ResourceLimits::memory(50 * 1024 * 1024);
	Magick::ResourceLimits::map(50 * 1024 * 1024);
	Magick::InitializeMagick(*argv);

	if (!token) { exit(1); }

	commands::diagnostics_set_boot();
	db::init();

	dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content | dpp::i_guild_members);
	bot.on_log(dpp::utility::cout_logger());

	bot.on_ready([&bot](const dpp::ready_t& event) {
		bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, "ponderin"));
		commands::register_all(bot);
		config::config_load();

		bot.start_timer([&bot](dpp::timer) { roles::role_sync(bot); }, 600);

		bot.start_timer([&bot](dpp::timer) { bazaar::b_refresh_all(bot); }, 3600);

		backup::do_backup();
		bazaar::b_refresh_all(bot);
		bot.start_timer([](dpp::timer) { backup::do_backup(); }, 86400);
	});

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) { commands::route_slash_command(bot, event); });

	bot.on_message_context_menu(
		[&bot](const dpp::message_context_menu_t& event) { commands::route_context_menu(bot, event); });

	bot.on_button_click([&bot](const dpp::button_click_t& event) { commands::route_button_click(bot, event); });

	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		if (event.msg.author.is_bot()) return;
		events::handle_message(event, bot);
	});

	bot.on_voice_state_update([&bot](const dpp::voice_state_update_t& event) { events::handle_voice(event, bot); });

	bot.start(dpp::st_wait);
	return 0;
}
