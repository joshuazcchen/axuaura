#include "commands.h"
#include <iostream>
#include "config.h"
#include "buttons.h"

namespace commands {
    std::map<std::string, SlashHandler> slash_map = {
        {"echo", handle_echo},
        {"auraboard", handle_auraboard},
        {"movaura", handle_movaura},
        {"duel", handle_duel},
    	{"settings", handle_settings},
    	{"gamble", handle_gamble},
        {"bet", handle_bet},
        {"smite", handle_smite},
        {"level", handle_level},
        {"fixlevel", handle_fixlevel},
        {"leaderboard", handle_leaderboard},
    };

    std::map<std::string, ContextHandler> context_map = {
    };

    void route_slash_command(dpp::cluster& bot, const dpp::slashcommand_t& event) {
        auto it = slash_map.find(event.command.get_command_name());
//	bot.message_create(dpp::message(config::LOG_CH, "<@" + std::to_string(event.command.usr.id) + "> slashcmd: " + event.command.get_command_name()));
        if (it != slash_map.end()) it->second(event, bot);
    }

    void route_context_menu(dpp::cluster& bot, const dpp::message_context_menu_t& event) {
        auto it = context_map.find(event.command.get_command_name());
//	bot.message_create(dpp::message(config::LOG_CH, "<@" + std::to_string(event.command.usr.id) + "> ctxm: " + event.command.get_command_name()));
        if (it != context_map.end()) it->second(event, bot);
    }

    void register_all(dpp::cluster& bot) {
        //buttons::register_handler("duel_accept_", handle_duel_buttons);
        //buttons::register_handler("duel_decline_", handle_duel_buttons);

        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_bulk_command_create({
                echo_def(bot),
                auraboard_def(bot),
                movaura_def(bot),
                duel_def(bot),
		        settings_def(bot),
		        gamble_def(bot),
                bet_def(bot),
                smite_def(bot),
                leaderboard_def(bot),
                fixlevel_def(bot),
                level_def(bot),
            });
        }
    }
}
