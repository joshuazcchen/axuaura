#include "commands.h"
#include "config.h"
#include "db.h"

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

	static std::string banner_filename(const std::string& data) {
		auto pos = data.find("\"file\"");
		if (pos == std::string::npos) return "";
		auto q1 = data.find('"', data.find(':', pos) + 1);
		auto q2 = data.find('"', q1 + 1);
		if (q1 == std::string::npos || q2 == std::string::npos) return "";
		return data.substr(q1 + 1, q2 - q1 - 1);
	}

	void handle_inventory(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sub = event.command.get_command_interaction().options[0].name;
		auto g_id = event.command.guild_id;
		auto u_id = event.command.get_issuing_user().id;

		if (sub == "view") {
			int page = std::get<int64_t>(event.get_parameter("page")) - 1;
			auto all = db::inv_get_user(g_id, u_id);
			int pgsz = config::BAZAAR_PGSZ;
			size_t start = std::min((size_t)page * pgsz, all.size());
			auto slice = std::span{all}.subspan(start, std::min((size_t)pgsz, all.size() - start));

			std::string out = "# your inventory\n\n";
			if (slice.empty()) {
				out += "*nothing here*";
			} else {
				int pos = start + 1;
				for (auto& i : slice) {
					std::string disp = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
					out += "**" + std::to_string(pos++) + ".** " + disp;
					if (i.equipped) out += " `[equipped]`";
					if (i.expires > 0) out += " *(expires <t:" + std::to_string(i.expires) + ":R>)*";
					out += "\n";
				}
			}
			out += "\n-# page " + std::to_string(page + 1) + " of " +
				   std::to_string(std::max(1, (int)((all.size() + pgsz - 1) / pgsz)));
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
				std::string fn = banner_filename(item.data);
				if (!fn.empty()) db::set_setting(g_id, "bg_override_" + std::to_string(u_id), fn);
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

			if (item.type == "role") {
				bot.guild_member_remove_role(g_id, u_id, item.role_id);
			} else if (item.type == "banner") {
				db::set_setting(g_id, "bg_override_" + std::to_string(u_id), "");
			}
			db::inv_uneq(g_id, u_id, inv.item_id);
			event.reply(dpp::message("unequipped **" + item.name + "**.")
							.set_allowed_mentions(false, false, false, false, {}, {}));
		}
	}

} // namespace commands
