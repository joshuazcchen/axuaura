#include <dpp/dpp.h>
#include <iostream>
#include <cstdlib>
#include "commands.h"
#include "db.h"

int main() {
    const char *token_env = std::getenv("BOT_TOKEN");
    if (!token_env) {
        std::cerr << "No token" << std::endl;
        exit(1);
    }

    db::init();

    dpp::cluster bot(token_env, dpp::i_default_intents | dpp::i_message_content);
    bot.on_log(dpp::utility::log_stderr());
    
    bot.on_ready([&bot](const dpp::ready_t &event) {
        std::cout << "version " << bot.me.format_full_name() << std::endl;
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
        // todo: this
    });

    bot.start(dpp::st_wait);
    return 0;
}
