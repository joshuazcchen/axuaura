#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "utils.h"

#include "image.h"
#include <atomic>
#include <malloc.h>

namespace commands {

	static std::atomic_bool inv_rendering{false};

	dpp::slashcommand inventory_def(dpp::cluster& bot) {
		return dpp::slashcommand("inventory", "view/manage your items", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "view", "view your inventory")
							.add_option(dpp::command_option(dpp::co_integer, "page", "page", false)))
			.add_option(
				dpp::command_option(dpp::co_sub_command, "equip", "equip an item")
					.add_option(dpp::command_option(dpp::co_integer, "id", "inventory position (1, 2, 3…)", true)))
			.add_option(
				dpp::command_option(dpp::co_sub_command, "unequip", "unequip an item")
					.add_option(dpp::command_option(dpp::co_integer, "id", "inventory position (1, 2, 3…)", true)));
	}

	void handle_inventory(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sub = event.command.get_command_interaction().options[0].name;
		auto g_id = event.command.guild_id;
		auto u_id = event.command.get_issuing_user().id;

		if (sub == "view") {
			event.thinking(true);

			auto page_param = event.get_parameter("page");
			int page = std::holds_alternative<int64_t>(page_param) ? (int)std::get<int64_t>(page_param) : 1;
			if (page < 1) page = 1;

			auto all = db::inv_get_user(g_id, u_id);
			constexpr int pgsz = 20;
			int total_pages = std::max(1, ((int)all.size() + pgsz - 1) / pgsz);
			if (page > total_pages) page = total_pages;

			int start = (page - 1) * pgsz;
			int end = std::min(start + pgsz, (int)all.size());
			std::vector<db::InvItem> slice(all.begin() + start, all.begin() + end);

			while (inv_rendering.exchange(true)) {}
			std::string img;
			try {
				std::string title_name = utils::get_display_name(g_id, u_id);
				std::string pfp_url = utils::get_avatar_url(u_id, 64);
				img = image::img_gen_inventory(slice, page, total_pages, title_name, pfp_url);
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

			auto build_btn_row = [&](int start, int count) {
				dpp::component row;
				row.set_type(dpp::cot_action_row);
				for (int i = start; i < start + count && i < (int)slice.size(); ++i) {
					const auto& it = slice[i];
					std::string lbl = it.name.size() > 10 ? it.name.substr(0, 9) + "\xe2\x80\xa6" : it.name;
					dpp::component btn;
					btn.set_type(dpp::cot_button)
						.set_id("inv_tog_" + std::to_string(page) + "_" + std::to_string(i + 1))
						.set_label(std::to_string(i + 1) + ". " + lbl)
						.set_style(it.equipped ? dpp::cos_success : dpp::cos_secondary);
					row.add_component(btn);
				}
				return row;
			};

			if (!slice.empty()) {
				int half = std::min(5, (int)slice.size());
				msg.add_component(build_btn_row(0, half));
				if ((int)slice.size() > 5) msg.add_component(build_btn_row(5, (int)slice.size() - 5));
			}

			event.edit_original_response(msg);
		} else if (sub == "equip") {
			int pos = std::get<int64_t>(event.get_parameter("id"));
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
			event.reply(dpp::message("equipped **" + item.name + "**.")
							.set_allowed_mentions(false, false, false, false, {}, {}));

		} else if (sub == "unequip") {
			int pos = std::get<int64_t>(event.get_parameter("id"));
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
			event.reply(dpp::message("unequipped **" + item.name + "**.")
							.set_allowed_mentions(false, false, false, false, {}, {}));
		}
	}

	void handle_inv_button(const dpp::button_click_t& event, dpp::cluster& bot) {
		const std::string& id = event.custom_id;
		std::string payload = id.substr(8);
		size_t sep = payload.find('_');
		if (sep == std::string::npos) return;

		int page, pos;
		try {
			page = std::stoi(payload.substr(0, sep));
			pos = std::stoi(payload.substr(sep + 1));
		} catch (...) { return; }

		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;

		auto items = db::inv_get_user(g_id, u_id);
		int idx = (page - 1) * 10 + pos - 1;
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
