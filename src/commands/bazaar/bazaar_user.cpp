#include <ctime>

#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "utils.h"

namespace commands {

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

		bool is_consumable = (item.type == "xp_boost");
		if (!is_consumable && db::inv_has(g_id, u_id, item_id)) {
			event.reply(dpp::message("you already own this.").set_flags(dpp::m_ephemeral));
			return;
		}

		int aura = db::get_aura(g_id, u_id);
		if (std::abs(aura) < std::abs(item.cost) ||
			(((aura > 0) - (aura < 0)) != ((item.cost > 0) - (item.cost < 0)))) {
			event.reply(
				dpp::message("not enough aura (" + std::to_string(aura) + "/" + std::to_string(item.cost) + ").")
					.set_flags(dpp::m_ephemeral));
			return;
		}

		db::rmv_aura(g_id, u_id, item.cost);

		if (item.type == "xp_boost") {
			int hours = 24;
			auto pos = item.data.find("\"hours\"");
			if (pos != std::string::npos) {
				auto colon = item.data.find(':', pos);
				if (colon != std::string::npos) try {
						hours = std::stoi(item.data.substr(colon + 1));
					} catch (...) {}
			}
			long expires = hours > 0 ? (std::time(nullptr) + (long)hours * 3600) : 0;
			db::inv_add_timed(g_id, u_id, item_id, expires);
		} else if (item.type == "banner") {
			std::string fn;
			auto pos = item.data.find("\"file\"");
			if (pos != std::string::npos) {
				auto q1 = item.data.find('"', item.data.find(':', pos) + 1);
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
		int refresh_hours = db::get_setting_int(g_id, "bazaar_refresh_hours", 168);

		std::string content = "# WELCOME TO TESCO\n\n";
		dpp::component pinned_row, rotating_row;
		pinned_row.set_type(dpp::cot_action_row);
		rotating_row.set_type(dpp::cot_action_row);
		bool had_pinned = false, had_rotating = false;
		long next_refresh = 0;

		for (auto& slot : slots) {
			auto item = db::shop_get(g_id, slot.item_id);
			if (item.item_id == -1) continue;
			std::string disp = (item.type == "role") ? "<@&" + std::to_string(item.role_id) + ">" : item.name;
			std::string line = "• **" + disp + "** — " + std::to_string(item.cost) + " aura";
			if (!item.desc.empty()) line += " | *" + item.desc + "*";

			dpp::component btn;
			btn.set_type(dpp::cot_button)
				.set_label(item.name.substr(0, 40) + " (" + std::to_string(item.cost) + " aura)")
				.set_id("bzr_buy_" + std::to_string(item.item_id));

			if (item.pinned) {
				if (!had_pinned) content += "**avail**\n";
				content += line + "\n";
				btn.set_style(dpp::cos_success);
				pinned_row.add_component(btn);
				had_pinned = true;
			} else {
				if (!had_rotating) content += "**rotating**\n";
				content += line + "\n";
				btn.set_style(dpp::cos_primary);
				rotating_row.add_component(btn);
				long deadline = slot.refreshed_at + (long)refresh_hours * 3600;
				if (next_refresh == 0 || deadline < next_refresh) next_refresh = deadline;
				had_rotating = true;
			}
		}

		if (!had_pinned && !had_rotating) content += "no items yet";
		if (next_refresh > 0) content += "\n-# rotation refreshes <t:" + std::to_string(next_refresh) + ":R>";

		dpp::message msg(ch_id, content);
		msg.set_allowed_mentions(false, false, false, false, {}, {});
		if (had_pinned && !pinned_row.components.empty()) msg.add_component(pinned_row);
		if (had_rotating && !rotating_row.components.empty()) msg.add_component(rotating_row);

		std::string id_str = db::get_setting_str(g_id, "bazaar_msg_id", "0");
		dpp::snowflake msg_id = std::stoull(id_str);
		if (msg_id != 0) {
			msg.id = msg_id;
			bot.message_edit(msg);
		} else {
			bot.message_create(msg, [g_id](const dpp::confirmation_callback_t& cb) {
				if (!cb.is_error())
					db::set_setting(g_id, "bazaar_msg_id", std::to_string(std::get<dpp::message>(cb.value).id));
			});
		}
	}

} // namespace bazaar
