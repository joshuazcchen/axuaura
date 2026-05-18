#pragma once
#include <dpp/dpp.h>

#include <functional>
#include <map>
#include <string>

namespace commands {
	using SlashHandler = std::function<void(const dpp::slashcommand_t&, dpp::cluster&)>;
	using ContextHandler = std::function<void(const dpp::message_context_menu_t&, dpp::cluster&)>;
	using ButtonHandler = std::function<void(const dpp::button_click_t&, dpp::cluster&)>;

	void register_all(dpp::cluster& bot);
	void route_slash_command(dpp::cluster& bot, const dpp::slashcommand_t& event);
	void route_context_menu(dpp::cluster& bot, const dpp::message_context_menu_t& event);
	void route_button_click(dpp::cluster& bot, const dpp::button_click_t& event);

	// slash cmds
	dpp::slashcommand echo_def(dpp::cluster& bot);
	void handle_echo(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand echoo_def(dpp::cluster& bot);
	void handle_echoo(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand mkspl_def(dpp::cluster& bot);
	void handle_mkspl(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand duel_def(dpp::cluster& bot);
	void handle_duel(const dpp::slashcommand_t& event, dpp::cluster& bot);
	// void handle_duel_buttons(const dpp::button_click_t& event, dpp::cluster& bot);
	// void process_duel_result(dpp::cluster& bot, dpp::snowflake channel_id, dpp::snowflake challenger_id,
	// dpp::snowflake opponent_id, int wager);

	dpp::slashcommand settings_def(dpp::cluster& bot);
	void handle_settings(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand gamble_def(dpp::cluster& bot);
	void handle_gamble(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand auraboard_def(dpp::cluster& bot);
	void handle_auraboard(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand movaura_def(dpp::cluster& bot);
	void handle_movaura(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand bet_def(dpp::cluster& bot);
	void handle_bet(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand smite_def(dpp::cluster& bot);
	void handle_smite(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand fixlevel_def(dpp::cluster& bot);
	void handle_fixlevel(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand leaderboard_def(dpp::cluster& bot);
	void handle_leaderboard(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand level_def(dpp::cluster& bot);
	void handle_level(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand inventory_def(dpp::cluster& bot);
	void handle_inventory(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand credits_def(dpp::cluster& bot);
	void handle_credits(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand coinflip_def(dpp::cluster& bot);
	void handle_coinflip(const dpp::slashcommand_t& event, dpp::cluster& bot);

	dpp::slashcommand diagnostics_def(dpp::cluster& bot);
	void handle_diagnostics(const dpp::slashcommand_t& event, dpp::cluster& bot);
	void diagnostics_set_boot();

	// ctxm DEPRECATED UNUSED
	dpp::slashcommand mkshm_def(dpp::cluster& bot);
	void handle_mkshm(const dpp::message_context_menu_t& event, dpp::cluster& bot);

	dpp::slashcommand mkshm_prime_def(dpp::cluster& bot);
	void handle_mkshm_prime(const dpp::message_context_menu_t& event, dpp::cluster& bot);

	dpp::slashcommand bazaar_def(dpp::cluster& bot);
	void handle_bazaar(const dpp::slashcommand_t& event, dpp::cluster& bot);
	void handle_bazaar_sell(const dpp::slashcommand_t& event, dpp::cluster& bot);
	void handle_bazaar_admin(const dpp::slashcommand_t& event, dpp::cluster& bot);

	// button handlers
	void handle_bazaar_button(const dpp::button_click_t& event, dpp::cluster& bot);
} // namespace commands
