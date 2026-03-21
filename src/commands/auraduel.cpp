#include "commands.h"
#include "db.h"
#include <random>
#include <ctime>

namespace commands {

    dpp::slashcommand get_auraduel_definition(dpp::cluster& bot) {
        return dpp::slashcommand("duel", "Challenge someone to a duel for aura", bot.me.id)
            .set_dm_permission(false)
            .add_option(dpp::command_option(dpp::co_user, "opponent", "who do you want to duel?", true))
            .add_option(dpp::command_option(dpp::co_integer, "wager", "how much aura to wager (min 10)", false));
    }

    void handle_auraduel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
        dpp::snowflake challenger_id = event.command.get_issuing_user().id;
        dpp::snowflake opponent_id = std::get<dpp::snowflake>(event.get_parameter("opponent"));
        int64_t wager = 50; // default wager

        if (event.get_parameter("wager")) {
            wager = std::get<int64_t>(event.get_parameter("wager"));
        }

        // Validation
        if (opponent_id == challenger_id) {
            event.reply(dpp::message("You can't duel yourself, lol").set_flags(dpp::m_ephemeral));
            return;
        }

        if (wager < 10) {
            event.reply(dpp::message("Minimum wager is 10 aura").set_flags(dpp::m_ephemeral));
            return;
        }

        int challenger_aura = db::get_aura(challenger_id);
        int opponent_aura = db::get_aura(opponent_id);

        if (challenger_aura < wager) {
            event.reply(dpp::message("You don't have enough aura to wager that much!").set_flags(dpp::m_ephemeral));
            return;
        }

        // Create duel invitation embed
        dpp::embed invite_embed = dpp::embed()
            .set_color(dpp::colors::red)
            .set_title("⚔️ DUEL CHALLENGE")
            .set_description(fmt::format("<@{}> challenges you to a duel!", challenger_id.str()))
            .add_field("Wager", std::to_string(wager) + " AURA", true)
            .add_field("Your Aura", std::to_string(opponent_aura) + " AURA", true)
            .add_field("Challenger Aura", std::to_string(challenger_aura) + " AURA", true)
            .set_footer(dpp::embed_footer()
                .set_text("Accept or decline the challenge below")
                .set_icon("https://cdn-icons-png.flaticon.com/512/595/595533.png"));

        // Create buttons
        dpp::message duel_msg(event.command.channel_id, invite_embed);
        duel_msg.add_component(
            dpp::component()
                .add_component(
                    dpp::component()
                        .set_type(dpp::cot_button)
                        .set_id(fmt::format("duel_accept_{}_{}", challenger_id.str(), opponent_id.str()))
                        .set_label("⚡ Accept")
                        .set_style(dpp::cos_success)
                )
                .add_component(
                    dpp::component()
                        .set_type(dpp::cot_button)
                        .set_id(fmt::format("duel_decline_{}_{}", challenger_id.str(), opponent_id.str()))
                        .set_label("✋ Decline")
                        .set_style(dpp::cos_danger)
                )
        );

        event.reply(duel_msg);
    }

    // Helper function to process duel results
    void process_duel_result(dpp::cluster& bot, dpp::snowflake challenger_id, dpp::snowflake opponent_id, int wager) {
        // Random duel outcome
        std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> distribution(0, 1);
        bool challenger_wins = distribution(rng);

        int challenger_aura = db::get_aura(challenger_id);
        int opponent_aura = db::get_aura(opponent_id);

        if (challenger_wins) {
            db::add_aura(challenger_id, wager);
            db::rmv_aura(opponent_id, wager);
        } else {
            db::add_aura(opponent_id, wager);
            db::rmv_aura(challenger_id, wager);
        }

        std::string winner = challenger_wins ? fmt::format("<@{}>", challenger_id.str()) : fmt::format("<@{}>", opponent_id.str());
        std::string loser = challenger_wins ? fmt::format("<@{}>", opponent_id.str()) : fmt::format("<@{}>", challenger_id.str());

        dpp::embed result_embed = dpp::embed()
            .set_color(challenger_wins ? dpp::colors::green : dpp::colors::red)
            .set_title("⚔️ DUEL RESULTS")
            .set_description(fmt::format("{} **WINS** against {}!", winner, loser))
            .add_field("Prize", fmt::format("+{} AURA", wager), true)
            .add_field("Loss", fmt::format("-{} AURA", wager), true);

        bot.current_user_get(
            [&bot, result_embed, challenger_id, opponent_id](const dpp::confirmation_callback_t& callback) {
                if (callback.is_error()) return;
            }
        );
    }
}

