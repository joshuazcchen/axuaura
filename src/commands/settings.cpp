// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "commands.h"
#include "config.h"
#include "db.h"

#include <nlohmann/json.hpp>

namespace commands {

	dpp::slashcommand settings_def(dpp::cluster& bot) {
		static const std::vector<std::string> setting_names = {
			"aurachancegain", "aurapassiveamt", "auralbozoamt",	   "auralossamt", "auralosscd",	 "status",	   "XP_MIN",
			"XP_MAX",		  "XP_COOLDOWN",	"levelup_channel", "sb_channel",  "sb_pos_cost", "sb_neg_cost"};

		dpp::command_option setting_option(dpp::co_string, "setting", "which one", true);
		for (const auto& name : setting_names) {
			setting_option.add_choice(dpp::command_option_choice(name, name));
		}

		return dpp::slashcommand("settings", "server settings", bot.me.id)
			.set_default_permissions(dpp::p_administrator)
			.add_option(dpp::command_option(dpp::co_sub_command, "config", "view or change a setting")
							.add_option(setting_option)
							.add_option(dpp::command_option(dpp::co_string, "value", "new value", false)))
			.add_option(dpp::command_option(dpp::co_sub_command, "levelrole", "assign a role to a level")
							.add_option(dpp::command_option(dpp::co_integer, "level", "level threshold", true))
							.add_option(dpp::command_option(dpp::co_role, "role", "role to grant", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "xpboostrole", "assign a role to an xp boost")
							.add_option(dpp::command_option(dpp::co_integer, "mult", "xp boost amt", true))
							.add_option(dpp::command_option(dpp::co_role, "role", "role to grant", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "aurarole", "assign a role to an aura rank slot")
							.add_option(dpp::command_option(dpp::co_string, "position", "which rank slot", true)
											.add_choice(dpp::command_option_choice("no1", std::string("no1")))
											.add_choice(dpp::command_option_choice("no2", std::string("no2")))
											.add_choice(dpp::command_option_choice("no3", std::string("no3")))
											.add_choice(dpp::command_option_choice("bot1", std::string("bot1")))
											.add_choice(dpp::command_option_choice("bot2", std::string("bot2")))
											.add_choice(dpp::command_option_choice("bot3", std::string("bot3"))))
							.add_option(dpp::command_option(dpp::co_role, "role", "role to assign", true)));
	}

	void handle_settings(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto cmd = event.command.get_command_interaction();
		std::string scmd = cmd.options[0].name;
		dpp::snowflake g_id = event.command.guild_id;

		if (scmd == "config") {
			std::string target_setting = std::get<std::string>(event.get_parameter("setting"));
			auto val_param = event.get_parameter("value");
			if (!std::holds_alternative<std::string>(val_param)) {
				std::string cur = db::get_setting_str(g_id, target_setting, "not set");
				event.reply(dpp::message(target_setting + " = `" + cur + "`").set_flags(dpp::m_ephemeral));
				return;
			}
			std::string new_val = std::get<std::string>(val_param);

			if (target_setting == "status") {
				db::set_setting(g_id, "status", new_val);
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
					if (target_setting == "XP_MIN")
						config::guild_configs[g_id].xp_min = rate;
					else if (target_setting == "XP_MAX")
						config::guild_configs[g_id].xp_max = rate;
					else if (target_setting == "XP_COOLDOWN")
						config::guild_configs[g_id].xp_cooldown = rate;
				} catch (...) {
					event.reply(dpp::message("invalid value, expected an int").set_flags(dpp::m_ephemeral));
					return;
				}
			}
			event.reply(dpp::message("done!").set_flags(dpp::m_ephemeral));

		} else if (scmd == "levelrole") {
			int64_t level = std::get<int64_t>(event.get_parameter("level"));
			dpp::snowflake role = std::get<dpp::snowflake>(event.get_parameter("role"));

			std::string json_str = db::get_setting_str(g_id, "xp_level_roles", "{}");
			try {
				nlohmann::json roles_map = nlohmann::json::parse(json_str);
				roles_map[std::to_string(level)] = std::to_string(role);
				db::set_setting(g_id, "xp_level_roles", roles_map.dump());
				event.reply(dpp::message("level " + std::to_string(level) + " > <@&" + std::to_string(role) + ">")
								.set_flags(dpp::m_ephemeral));
			} catch (...) { event.reply(dpp::message("failed to update level roles").set_flags(dpp::m_ephemeral)); }

		} else if (scmd == "xpboostrole") {
			int64_t boost = std::get<int64_t>(event.get_parameter("mult"));
			dpp::snowflake role = std::get<dpp::snowflake>(event.get_parameter("role"));

			std::string json_str = db::get_setting_str(g_id, "boost_roles", "{}");
			try {
				nlohmann::json roles_map = nlohmann::json::parse(json_str);
				roles_map[std::to_string(role)] = boost;
				db::set_setting(g_id, "boost_roles", roles_map.dump());
				event.reply(dpp::message("<@&" + std::to_string(role) + "> boost set to +" + std::to_string(boost))
								.set_flags(dpp::m_ephemeral));
			} catch (...) { event.reply(dpp::message("failed").set_flags(dpp::m_ephemeral)); }
		} else if (scmd == "aurarole") {
			std::string pos = std::get<std::string>(event.get_parameter("position"));
			dpp::snowflake role = std::get<dpp::snowflake>(event.get_parameter("role"));

			static const std::map<std::string, std::string> pos_to_key = {
				{"no1", "leader_role"}, {"no2", "num2_role"},  {"no3", "num3_role"},
				{"bot1", "loser_role"}, {"bot2", "bot2_role"}, {"bot3", "bot3_role"},
			};
			auto it = pos_to_key.find(pos);
			if (it == pos_to_key.end()) {
				event.reply(dpp::message("unknown position").set_flags(dpp::m_ephemeral));
				return;
			}
			db::set_setting(g_id, it->second, std::to_string(role));

			auto& conf = config::guild_configs[g_id];
			if (pos == "no1")
				conf.leader_role = role;
			else if (pos == "no2")
				conf.num2_role = role;
			else if (pos == "no3")
				conf.num3_role = role;
			else if (pos == "bot1")
				conf.loser_role = role;
			else if (pos == "bot2")
				conf.bot2_role = role;
			else if (pos == "bot3")
				conf.bot3_role = role;
			conf.update_stupid();

			event.reply(dpp::message(pos + " > <@&" + std::to_string(role) + ">").set_flags(dpp::m_ephemeral));
		}
	}

} // namespace commands
