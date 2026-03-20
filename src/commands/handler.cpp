#include "commands.h"
#include <iostream>

namespace commands {
    std::map<std::string, SlashHandler> slash_map = {
        {"echo", handle_echo},
        {"mkspl", handle_mkspl},
        {"auraduel", handle_auraduel},
        {"auraboard", handle_auraboard},
        {"movaura", handle_movaura}
    };
    
    std::map<std::string, ContextHandler> context_map = {
        {"mkshm", handle_mkshm},
        {"mkshm_prime", handle_mkshm_prime}
    };

    void route_slash_command(dpp::cluster& bot, const dpp::slashcommand_t& event) {
        auto it = slash_map.find(event.command.get_command_name());
        if (it != slash_map.end()) it->second(event, bot);
    }

    void route_context_menu(dpp::cluster& bot, const dpp::message_context_menu_t& event) {
        auto it = context_map.find(event.command.get_command_name());
        if (it != context_map.end()) it->second(event, bot);
    }

    void register_all(dpp::cluster& bot) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_bulk_command_create({
                get_echo_definition(bot),
                get_echoo_definition(bot),
                get_mkspl_definition(bot),
                get_auraduel_definition(bot),
                get_auraboard_definition(bot),
                get_movaura_definition(bot),
                get_mkshm_definition(bot),
                get_mkshm_prime_definition(bot)
            });
        }
    }
}
