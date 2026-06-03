#include <iostream>

#include "commands.h"
#include "config.h"

namespace commands {

	std::map<std::string, SlashHandler> slash_map = {
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
		{"bazaar", handle_bazaar},
		{"inventory", handle_inventory},
		{"credits", handle_credits},
		{"coinflip", handle_coinflip},
		{"diagnostics", handle_diagnostics},
		{"echo", handle_echo},
		{"aura", handle_aura},
		{"setlevel", handle_setlevel},
		{"starboard", handle_starboard_admin},
	};

	std::map<std::string, ContextHandler> context_map = {
		{"mkshm", handle_mkshm},
		{"giveitm", handle_giveitm},
		{"starboard", handle_starboard_ctx},
	};

	std::map<std::string, ButtonHandler> button_map = {
		{"bzr_buy_", handle_bazaar_button},
		{"sb_pos_", handle_starboard_button},
		{"sb_neg_", handle_starboard_button},
		{"inv_tog_", handle_inv_button},
	};

	void route_slash_command(dpp::cluster& bot, const dpp::slashcommand_t& event) {
		auto it = slash_map.find(event.command.get_command_name());
		if (it != slash_map.end()) it->second(event, bot);
	}

	void route_context_menu(dpp::cluster& bot, const dpp::message_context_menu_t& event) {
		auto it = context_map.find(event.command.get_command_name());
		if (it != context_map.end()) it->second(event, bot);
	}

	void route_button_click(dpp::cluster& bot, const dpp::button_click_t& event) {
		const std::string& id = event.custom_id;
		for (const auto& [prefix, handler] : button_map) {
			if (id.rfind(prefix, 0) == 0) {
				handler(event, bot);
				return;
			}
		}
	}

	void register_all(dpp::cluster& bot) {
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.global_bulk_command_create({
				auraboard_def(bot),		  movaura_def(bot),		  duel_def(bot),		settings_def(bot),
				gamble_def(bot),		  bet_def(bot),			  smite_def(bot),		leaderboard_def(bot),
				fixlevel_def(bot),		  level_def(bot),		  bazaar_def(bot),		inventory_def(bot),
				credits_def(bot),		  coinflip_def(bot),	  diagnostics_def(bot), echo_def(bot),
				setlevel_def(bot),		  mkshm_def(bot),		  giveitm_def(bot),		aura_def(bot),
				starboard_admin_def(bot), starboard_ctx_def(bot),
			});
		}
	}

} // namespace commands
