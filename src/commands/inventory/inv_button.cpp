// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "inventory.h"

namespace commands {

	void handle_inv_button(const dpp::button_click_t& event, dpp::cluster& bot) {
		const std::string& id = event.custom_id;
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;

		// page navigation
		if (id.rfind("inv_prev_", 0) == 0 || id.rfind("inv_next_", 0) == 0) {
			bool is_prev = id.rfind("inv_prev_", 0) == 0;
			int cur_page;
			try {
				cur_page = std::stoi(id.substr(9));
			} catch (...) { return; }
			int new_page = is_prev ? cur_page - 1 : cur_page + 1;

			event.thinking(true);

			auto all = db::inv_get_user(g_id, u_id);
			int total_pages = std::max(1, ((int)all.size() + INV_PGSZ - 1) / INV_PGSZ);
			if (new_page < 1) new_page = 1;
			if (new_page > total_pages) new_page = total_pages;

			int start = (new_page - 1) * INV_PGSZ;
			std::vector<db::InvItem> slice(all.begin() + start,
										   all.begin() + std::min(start + INV_PGSZ, (int)all.size()));

			while (inv_rendering.exchange(true)) {}
			std::string img;
			try {
				img = image::img_gen_inventory(slice, new_page, total_pages, utils::get_display_name(g_id, u_id),
											   utils::get_avatar_url(u_id, 64));
			} catch (...) {}
			inv_rendering = false;
			malloc_trim(0);

			if (img.empty()) {
				event.edit_original_response(dpp::message("couldn't render inventory.").set_flags(dpp::m_ephemeral));
				return;
			}

			dpp::message msg(event.command.channel_id, "");
			msg.set_flags(dpp::m_ephemeral);
			msg.add_file("inventory.png", img);
			add_inv_buttons(msg, slice, new_page, total_pages);
			event.edit_original_response(msg);
			return;
		}

		// equip / unequip toggle
		std::string payload = id.substr(8); // strip "inv_tog_"
		size_t sep = payload.find('_');
		if (sep == std::string::npos) return;

		int page, pos;
		try {
			page = std::stoi(payload.substr(0, sep));
			pos = std::stoi(payload.substr(sep + 1));
		} catch (...) { return; }

		auto items = db::inv_get_user(g_id, u_id);
		int idx = (page - 1) * INV_PGSZ + pos - 1;
		if (idx < 0 || idx >= (int)items.size()) {
			event.reply(dpp::message("item not found.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto& inv = items[idx];
		auto item = db::shop_get(g_id, inv.item_id);

		if (item.type == "xp_boost") {
			event.reply(dpp::message("xp boosters can't be manually unequipped").set_flags(dpp::m_ephemeral));
			return;
		}

		if (inv.equipped) {
			if (item.type == "role") bot.guild_member_remove_role(g_id, u_id, item.role_id);
			if (item.type == "banner") bazaar::b_banner_unequip(g_id, u_id);
			db::inv_uneq(g_id, u_id, inv.item_id);
			event.reply(dpp::message("unequipped **" + item.name + "**.")
							.set_flags(dpp::m_ephemeral)
							.set_allowed_mentions(false, false, false, false, {}, {}));
		} else {
			if (item.type == "role") bot.guild_member_add_role(g_id, u_id, item.role_id);
			if (item.type == "banner") {
				db::inv_uneq_all_type(g_id, u_id, "banner");
				bazaar::b_banner_equip(g_id, u_id, item);
			}
			db::inv_eq(g_id, u_id, inv.item_id);
			event.reply(dpp::message("equipped **" + item.name + "**.")
							.set_flags(dpp::m_ephemeral)
							.set_allowed_mentions(false, false, false, false, {}, {}));
		}
	}

} // namespace commands
