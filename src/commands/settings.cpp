// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "commands.h"
#include "config.h"
#include "db.h"

namespace commands {

	dpp::slashcommand settings_def(dpp::cluster& bot) {
		return dpp::slashcommand("settings", "set ings", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			// TODO: make better
			.add_option(dpp::command_option(dpp::co_string, "setting", "which one", true)
							.add_choice(dpp::command_option_choice("aurachancegain", std::string("aurachancegain")))
							.add_choice(dpp::command_option_choice("aurapassiveamt", std::string("aurapassiveamt")))
							.add_choice(dpp::command_option_choice("auralbozoamt", std::string("auralbozoamt")))
							.add_choice(dpp::command_option_choice("auralossamt", std::string("auralbozoamt")))
							.add_choice(dpp::command_option_choice("auralosscd", std::string("auralosscd")))
							.add_choice(dpp::command_option_choice("status", std::string("status")))
							.add_choice(dpp::command_option_choice("XP_MIN", std::string("XP_MIN")))
							.add_choice(dpp::command_option_choice("XP_MAX", std::string("XP_MAX")))
							.add_choice(dpp::command_option_choice("XP_COOLDOWN", std::string("XP_COOLDOWN")))
							.add_choice(dpp::command_option_choice("levelup_channel", std::string("levelup_channel")))
							.add_choice(dpp::command_option_choice("sb_channel", std::string("sb_channel")))
							.add_choice(dpp::command_option_choice("sb_pos_cost", std::string("sb_pos_cost")))
							.add_choice(dpp::command_option_choice("sb_neg_cost", std::string("sb_neg_cost"))))
			.add_option(dpp::command_option(dpp::co_string, "value", "val"));
	}

	void handle_settings(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		std::string target_setting = std::get<std::string>(event.get_parameter("setting"));
		dpp::snowflake g_id = event.command.guild_id;

		auto val_param = event.get_parameter("value");
		if (!std::holds_alternative<std::string>(val_param)) {
			std::string cur = db::get_setting_str(g_id, target_setting, "not set");
			event.reply(dpp::message(target_setting + " = `" + cur + "`").set_flags(dpp::m_ephemeral));
			return;
		}

		std::string new_val = std::get<std::string>(val_param);

		if (target_setting == "status") {
			db::set_setting(event.command.guild_id, "status", new_val);
			bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game, new_val));
		} else if (target_setting == "levelup_channel" || target_setting == "sb_channel") {
			std::string clean = new_val;
			if (clean.size() > 3 && clean[0] == '<' && clean[1] == '#' && clean.back() == '>')
				clean = clean.substr(2, clean.size() - 3);
			try {
				dpp::snowflake ch_id = std::stoull(clean);
				std::string db_key = (target_setting == "levelup_channel") ? "lvl_ch" : "sb_channel";
				db::set_setting(g_id, db_key, std::to_string(ch_id));
				if (target_setting == "levelup_channel") config::guild_configs[g_id].lvl_ch = ch_id;
			} catch (...) {
				event.reply(dpp::message("provide a valid channel ID or mention.").set_flags(dpp::m_ephemeral));
				return;
			}

		} else if (target_setting == "sb_pos_cost" || target_setting == "sb_neg_cost") {
			try {
				int cost = std::stoi(new_val);
				if (cost < 0) {
					event.reply(dpp::message("cost must be >= 0.").set_flags(dpp::m_ephemeral));
					return;
				}
				db::set_setting(g_id, target_setting, cost);
			} catch (...) {
				event.reply(dpp::message("invalid value, expected a positive int").set_flags(dpp::m_ephemeral));
				return;
			}
		} else {
			try {
				int rate = std::stoi(new_val);
				db::set_setting(g_id, target_setting, rate);

				if (target_setting == "XP_MIN") {
					config::guild_configs[g_id].xp_min = rate;
				} else if (target_setting == "XP_MAX") {
					config::guild_configs[g_id].xp_max = rate;
				} else if (target_setting == "XP_COOLDOWN") {
					config::guild_configs[g_id].xp_cooldown = rate;
				}
			} catch (...) {
				event.reply(dpp::message("invalid value, expected an int").set_flags(dpp::m_ephemeral));
				return;
			}
		}

		event.reply(dpp::message("your did it!").set_flags(dpp::m_ephemeral));
	}

} // namespace commands
