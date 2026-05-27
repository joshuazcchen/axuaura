#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "utils.h"

namespace commands {

	dpp::slashcommand inventory_def(dpp::cluster& bot) {
		return dpp::slashcommand("inventory", "view/manage your items", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "view", "view your inventory")
							.add_option(dpp::command_option(dpp::co_integer, "page", "page", true)))
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
			int page = std::get<int64_t>(event.get_parameter("page")) - 1;
			auto all = db::inv_get_user(g_id, u_id);
			int pgsz = config::BAZAAR_PGSZ;
			int start = std::min((int)page * pgsz, (int)all.size());
			int end = std::min(start + pgsz, (int)all.size());

			std::string out = "# your inventory\n\n";
			if (start == (int)all.size()) {
				out += "*nothing here*";
			} else {
				for (int i = start; i < end; ++i) {
					auto& item = all[i];
					std::string disp = (item.type == "role") ? "<@&" + std::to_string(item.role_id) + ">" : item.name;
					out += "**" + std::to_string(i + 1) + ".** " + disp;
					if (item.equipped) out += " `[equipped]`";
					if (item.expires > 0) out += " *(expires <t:" + std::to_string(item.expires) + ":R>)*";
					out += "\n";
				}
			}
			out += "\n-# page " + std::to_string(page + 1) + " of " +
				   std::to_string(std::max(1, ((int)all.size() + pgsz - 1) / pgsz));
			event.reply(dpp::message(out).set_allowed_mentions(false, false, false, false, {}, {}));

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

} // namespace commands
