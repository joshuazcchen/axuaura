#include <dpp/dpp.h>
#include <iostream>
#include <cstdlib>
#include <string>
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
    bot.on_ready([&bot](const dpp::ready_t &event) {
        std::cout << "version " << bot.me.format_username() << std::endl;
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

    // Handle button interactions for duel
    bot.on_button_click([&bot](const dpp::button_click_t& event) {
        if (event.custom_id.find("duel_") != 0) return; // Not a duel button

        // Parse button ID format: "duel_accept_<challenger_id>_<opponent_id>" or "duel_decline_..."
        std::string button_type = event.custom_id.substr(5, event.custom_id.find('_', 5) - 5); // "accept" or "decline"
        
        // Extract IDs from custom_id
        size_t first_underscore = event.custom_id.find('_', 5);
        size_t second_underscore = event.custom_id.rfind('_');
        
        dpp::snowflake challenger_id(event.custom_id.substr(first_underscore + 1, second_underscore - first_underscore - 1));
        dpp::snowflake opponent_id(event.custom_id.substr(second_underscore + 1));

        // Check if the button clicker is the opponent
        if (event.command.usr.id != opponent_id) {
            event.reply(dpp::message("Only <@" + opponent_id.str() + "> can respond to this duel!").set_flags(dpp::m_ephemeral));
            return;
        }

        if (button_type == "accept") {
            // Extract wager from message (we'll need to capture it somewhere)
            // For now, default wager of 50
            int wager = 50;
            
            // Parse wager from embed if present
            if (!event.command.resolved.messages.empty()) {
                auto msg = event.command.resolved.messages.begin()->second;
                if (!msg.embeds.empty()) {
                    auto embed = msg.embeds.front();
                    for (const auto& field : embed.fields) {
                        if (field.name == "Wager") {
                            // Extract number from "50 AURA" format
                            wager = std::stoi(field.value.substr(0, field.value.find(' ')));
                            break;
                        }
                    }
                }
            }

            commands::process_duel_result(bot, challenger_id, opponent_id, wager);
            event.reply(dpp::message("Duel accepted! ⚔️").set_flags(dpp::m_ephemeral));
        } else if (button_type == "decline") {
            event.reply(dpp::message("<@" + event.command.usr.id.str() + "> declined the duel.").set_flags(dpp::m_ephemeral));
        }
    });

    bot.start(dpp::st_wait);
    return 0;
}
