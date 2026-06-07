// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "inventory.h"

namespace commands {

	void handle_inv_equip(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto g_id = event.command.guild_id;
		auto u_id = event.command.get_issuing_user().id;
		int pos = (int)std::get<int64_t>(event.get_parameter("id"));
		auto items = db::inv_get_user(g_id, u_id);
		if (pos < 1 || pos > (int)items.size()) {
			event.reply(dpp::message("invalid position.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto& inv = items[pos - 1];
		auto item = db::shop_get(g_id, inv.item_id);

		if (item.type == "role") {
			bot.guild_member_add_role(g_id, u_id, item.role_id);
		} else if (item.type == "banner") {
			db::inv_uneq_all_type(g_id, u_id, "banner");
			bazaar::b_banner_equip(g_id, u_id, item);
		}
		db::inv_eq(g_id, u_id, inv.item_id);
		event.reply(
			dpp::message("equipped **" + item.name + "**.").set_allowed_mentions(false, false, false, false, {}, {}));
	}

	void handle_inv_unequip(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto g_id = event.command.guild_id;
		auto u_id = event.command.get_issuing_user().id;
		int pos = (int)std::get<int64_t>(event.get_parameter("id"));
		auto items = db::inv_get_user(g_id, u_id);
		if (pos < 1 || pos > (int)items.size()) {
			event.reply(dpp::message("invalid position.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto& inv = items[pos - 1];
		auto item = db::shop_get(g_id, inv.item_id);

		if (item.type == "xp_boost") {
			event.reply(dpp::message("xp boosters can't be unequipped").set_flags(dpp::m_ephemeral));
			return;
		}
		if (item.type == "role") bot.guild_member_remove_role(g_id, u_id, item.role_id);
		if (item.type == "banner") bazaar::b_banner_unequip(g_id, u_id);
		db::inv_uneq(g_id, u_id, inv.item_id);
		event.reply(
			dpp::message("unequipped **" + item.name + "**.").set_allowed_mentions(false, false, false, false, {}, {}));
	}

} // namespace commands
