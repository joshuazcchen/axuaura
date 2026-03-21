#pragma once
#include <dpp/dpp.h>
#include <map>
#include <string>
#include <functional>

namespace commands {
    using SlashHandler = std::function<void(const dpp::slashcommand_t&, dpp::cluster&)>;
    using ContextHandler = std::function<void(const dpp::message_context_menu_t&, dpp::cluster&)>;

    void register_all(dpp::cluster& bot);
    void route_slash_command(dpp::cluster& bot, const dpp::slashcommand_t& event);
    void route_context_menu(dpp::cluster& bot, const dpp::message_context_menu_t& event);

    // slash cmds
    dpp::slashcommand get_echo_definition(dpp::cluster& bot);
    void handle_echo(const dpp::slashcommand_t& event, dpp::cluster& bot);

    dpp::slashcommand get_echoo_definition(dpp::cluster& bot);
    void handle_echoo(const dpp::slashcommand_t& event, dpp::cluster& bot);

    dpp::slashcommand get_mkspl_definition(dpp::cluster& bot);
    void handle_mkspl(const dpp::slashcommand_t& event, dpp::cluster& bot);

    dpp::slashcommand get_auraduel_definition(dpp::cluster& bot);
    void handle_auraduel(const dpp::slashcommand_t& event, dpp::cluster& bot);
    void process_duel_result(dpp::cluster& bot, dpp::snowflake channel_id, dpp::snowflake challenger_id, dpp::snowflake opponent_id, int wager);

    dpp::slashcommand get_settings_definition(dpp::cluster& bot);
    void handle_settings(const dpp::slashcommand_t& event, dpp::cluster&bot);

    dpp::slashcommand get_auraboard_definition(dpp::cluster& bot);
    void handle_auraboard(const dpp::slashcommand_t& event, dpp::cluster& bot);

    dpp::slashcommand get_movaura_definition(dpp::cluster& bot);
    void handle_movaura(const dpp::slashcommand_t& event, dpp::cluster& bot);

    // ctxm
    dpp::slashcommand get_mkshm_definition(dpp::cluster& bot);
    void handle_mkshm(const dpp::message_context_menu_t& event, dpp::cluster& bot);

    dpp::slashcommand get_mkshm_prime_definition(dpp::cluster& bot);
    void handle_mkshm_prime(const dpp::message_context_menu_t& event, dpp::cluster& bot);
}
