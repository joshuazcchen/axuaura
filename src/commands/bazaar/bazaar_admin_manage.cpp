#include <algorithm>

#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "utils.h"

namespace commands {
	bool handle_admin_item_cmd(const std::string& sub, const dpp::slashcommand_t& event, dpp::cluster& bot);

	void handle_bazaar_admin(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		if (!utils::is_admin(event)) {
			event.reply(dpp::message("no.").set_flags(dpp::m_ephemeral));
			return;
		}
		dpp::snowflake g_id = event.command.guild_id;
		auto subcmd = event.command.get_command_interaction().options[0].options[0];
		const std::string& sub = subcmd.name;

		if (handle_admin_item_cmd(sub, event, bot)) return;

		auto adm_ok = [&] { event.reply(dpp::message("done.").set_flags(dpp::m_ephemeral)); };
		auto adm_err = [&](const std::string& s) { event.reply(dpp::message(s).set_flags(dpp::m_ephemeral)); };

		if (sub == "give") {
			dpp::snowflake u = std::get<dpp::snowflake>(event.get_parameter("user"));
			int id = std::get<int64_t>(event.get_parameter("id"));
			db::inv_add(g_id, u, id);
			adm_ok();

		} else if (sub == "take") {
			dpp::snowflake u = std::get<dpp::snowflake>(event.get_parameter("user"));
			int id = std::get<int64_t>(event.get_parameter("id"));
			auto p_rel = event.get_parameter("relative");
			bool relative = std::holds_alternative<bool>(p_rel) && std::get<bool>(p_rel);
			if (relative) {
				auto items = db::inv_get_user(g_id, u);
				if (id < 1 || id > (int)items.size()) {
					adm_err("invalid position");
					return;
				}
				db::inv_rm_by_inv_id(items[id - 1].inv_id);
			} else {
				db::inv_rm(g_id, u, id);
			}
			adm_ok();

		} else if (sub == "viewinv") {
			dpp::snowflake u = std::get<dpp::snowflake>(event.get_parameter("user"));
			int page = std::get<int64_t>(event.get_parameter("page")) - 1;
			auto all = db::inv_get_user(g_id, u);
			int pgsz = config::BAZAAR_PGSZ;
			size_t start = std::min((size_t)page * pgsz, all.size());
			auto slice = std::span{all}.subspan(start, std::min((size_t)pgsz, all.size() - start));

			std::string out = "# inventory of <@" + std::to_string(u) + ">\n\n";
			if (slice.empty()) {
				out += "*nothing*";
			} else {
				int pos = start + 1;
				for (auto& i : slice) {
					std::string display = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
					out += "**" + std::to_string(pos++) + ".** " + display + (i.equipped ? " `[equipped]`" : "") +
						   (i.expires > 0 ? " *(expires <t:" + std::to_string(i.expires) + ":R>)*" : "") + "\n";
				}
			}
			out += "-# page " + std::to_string(page + 1) + " of " + std::to_string((all.size() + pgsz - 1) / pgsz);
			event.reply(dpp::message(out).set_allowed_mentions(false, false, false, false, {}, {}));

		} else if (sub == "reroll") {
			int slot = std::get<int64_t>(event.get_parameter("slot"));
			db::bazaar_rotation_clear_slot(g_id, slot);
			bazaar::b_refresh_guild(bot, g_id);
			adm_ok();

		} else if (sub == "setchannel") {
			dpp::snowflake ch = std::get<dpp::snowflake>(event.get_parameter("channel"));
			db::set_setting(g_id, "bazaar_channel", std::to_string(ch));
			db::set_setting(g_id, "bazaar_msg_id", std::string("0"));
			bazaar::b_refresh_guild(bot, g_id);
			event.reply(dpp::message("bazaar channel set. make sure nobody else can send messages there.")
							.set_flags(dpp::m_ephemeral));

		} else if (sub == "setrotation") {
			auto p_pos = event.get_parameter("positive");
			auto p_neg = event.get_parameter("negative");
			auto p_hrs = event.get_parameter("refresh_hours");
			if (std::holds_alternative<int64_t>(p_pos))
				db::set_setting(g_id, "bazaar_positive_cnt", (int)std::get<int64_t>(p_pos));
			if (std::holds_alternative<int64_t>(p_neg))
				db::set_setting(g_id, "bazaar_negative_cnt", (int)std::get<int64_t>(p_neg));
			if (std::holds_alternative<int64_t>(p_hrs))
				db::set_setting(g_id, "bazaar_refresh_hours", (int)std::get<int64_t>(p_hrs));
			adm_ok();

		} else if (sub == "refresh") {
			db::set_setting(g_id, "bazaar_msg_id", std::string("0"));
			bazaar::b_refresh_guild(bot, g_id);
			adm_ok();

		} else if (sub == "listall") {
			int page = std::get<int64_t>(event.get_parameter("page")) - 1;
			auto all = db::shop_get_all(g_id, false);
			int pgsz = config::BAZAAR_PGSZ;
			size_t start = std::min((size_t)page * pgsz, all.size());
			auto slice = std::span{all}.subspan(start, std::min((size_t)pgsz, all.size() - start));

			std::string out = "";
			for (auto& i : slice) {
				std::string disp = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
				out += std::to_string(i.item_id) + " [" + i.type + "] " + disp + " " + std::to_string(i.cost) + "aura" +
					   (i.active ? " ACTIVE" : " INACTIVE") + (i.pinned ? " PINNED" : "") +
					   (i.global ? " GLOBAL" : "") + "\n";
			}
			if (out.empty()) out = "no items";
			event.reply(dpp::message(out).set_allowed_mentions(false, false, false, false, {}, {}));
		}
	}

} // namespace commands
