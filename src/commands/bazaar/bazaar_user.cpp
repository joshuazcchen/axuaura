#include <ctime>

#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "image.h"
#include "utils.h"

namespace commands {

	// ── sell (by inventory position) ────────────────────────────────────────
	void handle_bazaar_sell(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		int pos = std::get<int64_t>(event.get_parameter("id"));

		auto items = db::inv_get_user(g_id, u_id);
		if (pos < 1 || pos > (int)items.size()) {
			event.reply(dpp::message("invalid position.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto& inv = items[pos - 1];
		if (db::shop_state(g_id, inv.item_id, "sellability") == 0) {
			event.reply(dpp::message("can't sell that one.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto item = db::shop_get(g_id, inv.item_id);
		int refund = std::max(0, item.cost / 10);

		if (item.type == "role") bot.guild_member_remove_role(g_id, u_id, item.role_id);
		if (item.type == "banner" && inv.equipped) db::set_setting(g_id, "bg_override_" + std::to_string(u_id), "");

		db::add_aura(g_id, u_id, refund);
		db::inv_rm_by_inv_id(inv.inv_id);
		event.reply(dpp::message("sold **" + item.name + "** for " + std::to_string(refund) + " aura."));
	}

	// ── buy button handler ───────────────────────────────────────────────────
	void handle_bazaar_button(const dpp::button_click_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		int item_id = std::stoi(event.custom_id.substr(8));

		auto slots = db::bazaar_rotation_get(g_id);
		bool in_rotation = false;
		for (auto& s : slots)
			if (s.item_id == item_id) {
				in_rotation = true;
				break;
			}
		if (!in_rotation) {
			event.reply(dpp::message("this item is no longer available.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto item = db::shop_get(g_id, item_id);
		if (item.item_id == -1 || !item.active) {
			event.reply(dpp::message("item unavailable.").set_flags(dpp::m_ephemeral));
			return;
		}

		bool consumable = (item.type == "xp_boost");
		if (!consumable && db::inv_has(g_id, u_id, item_id)) {
			event.reply(dpp::message("you already own this.").set_flags(dpp::m_ephemeral));
			return;
		}

		int aura = db::get_aura(g_id, u_id);
		bool can_buy = (item.cost >= 0) ? (aura >= item.cost) : (aura <= item.cost);
		if (!can_buy) {
			event.reply(dpp::message("not enough aura.").set_flags(dpp::m_ephemeral));
			return;
		}

		db::rmv_aura(g_id, u_id, item.cost);

		if (item.type == "xp_boost") {
			int hours = 24;
			auto p = item.data.find("\"hours\"");
			if (p != std::string::npos) {
				auto c = item.data.find(':', p);
				if (c != std::string::npos) try {
						hours = std::stoi(item.data.substr(c + 1));
					} catch (...) {}
			}
			long expires = hours > 0 ? (std::time(nullptr) + (long)hours * 3600) : 0;
			db::inv_add_timed(g_id, u_id, item_id, expires);
		} else if (item.type == "banner") {
			std::string fn;
			auto p = item.data.find("\"file\"");
			if (p != std::string::npos) {
				auto q1 = item.data.find('"', item.data.find(':', p) + 1);
				auto q2 = item.data.find('"', q1 + 1);
				if (q1 != std::string::npos && q2 != std::string::npos) fn = item.data.substr(q1 + 1, q2 - q1 - 1);
			}
			db::inv_uneq_all_type(g_id, u_id, "banner");
			db::inv_add(g_id, u_id, item_id);
			db::inv_eq(g_id, u_id, item_id);
			if (!fn.empty()) db::set_setting(g_id, "bg_override_" + std::to_string(u_id), fn);
		} else {
			db::inv_add(g_id, u_id, item_id);
			db::inv_eq(g_id, u_id, item_id);
			if (item.type == "role") bot.guild_member_add_role(g_id, u_id, item.role_id);
		}
		event.reply(dpp::message("purchased **" + item.name + "**!").set_flags(dpp::m_ephemeral));
	}

} // namespace commands

namespace bazaar {

	void b_post_ui(dpp::cluster& bot, dpp::snowflake g_id) {
		std::string ch_str = db::get_setting_str(g_id, "bazaar_channel", "0");
		dpp::snowflake ch_id = std::stoull(ch_str);
		if (ch_id == 0) return;

		auto slots = db::bazaar_rotation_get(g_id);

		std::vector<db::ShopItem> pos_items, neg_items;
		for (auto& s : slots) {
			auto item = db::shop_get(g_id, s.item_id);
			if (item.item_id == -1 || !item.active) continue;
			(item.cost >= 0 ? pos_items : neg_items).push_back(item);
		}

		std::string img_data = image::img_gen_bazaar(pos_items, neg_items);
		if (img_data.empty()) return;

		dpp::component pos_row, neg_row;
		pos_row.set_type(dpp::cot_action_row);
		neg_row.set_type(dpp::cot_action_row);
		for (auto& i : pos_items) {
			std::string lbl = i.name.substr(0, 18) + " (" + std::to_string(i.cost) + "a)";
			dpp::component btn;
			btn.set_type(dpp::cot_button)
				.set_style(dpp::cos_success)
				.set_label(lbl)
				.set_id("bzr_buy_" + std::to_string(i.item_id));
			pos_row.add_component(btn);
		}
		for (auto& i : neg_items) {
			std::string lbl = i.name.substr(0, 18) + " (" + std::to_string(i.cost) + "a)";
			dpp::component btn;
			btn.set_type(dpp::cot_button)
				.set_style(dpp::cos_danger)
				.set_label(lbl)
				.set_id("bzr_buy_" + std::to_string(i.item_id));
			neg_row.add_component(btn);
		}

		dpp::message msg(ch_id, "");
		msg.add_file("bazaar.png", img_data, "image/png");
		msg.set_allowed_mentions(false, false, false, false, {}, {});
		if (!pos_items.empty()) msg.add_component(pos_row);
		if (!neg_items.empty()) msg.add_component(neg_row);

		std::string id_str = db::get_setting_str(g_id, "bazaar_msg_id", std::string("0"));
		dpp::snowflake old_id = std::stoull(id_str);
		if (old_id != 0) bot.message_delete(old_id, ch_id);

		db::set_setting(g_id, "bazaar_msg_id", std::string("0"));
		bot.message_create(msg, [g_id](const dpp::confirmation_callback_t& cb) {
			if (!cb.is_error())
				db::set_setting(g_id, "bazaar_msg_id", std::to_string(std::get<dpp::message>(cb.value).id));
		});
	}

} // namespace bazaar
