#include <filesystem>

#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "utils.h"

namespace commands {

	static dpp::message adm_ok() { return dpp::message("done.").set_flags(dpp::m_ephemeral); }
	static dpp::message adm_err(const std::string& s) { return dpp::message(s).set_flags(dpp::m_ephemeral); }

	static void do_add(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		std::string type = std::get<std::string>(event.get_parameter("type"));
		int cost = std::get<int64_t>(event.get_parameter("cost"));

		std::string name, desc, data;
		dpp::snowflake r_id = 0;
		bool pinned = false;

		auto p_desc = event.get_parameter("desc");
		auto p_name = event.get_parameter("name");
		auto p_pinned = event.get_parameter("pinned");
		if (std::holds_alternative<std::string>(p_desc)) desc = std::get<std::string>(p_desc);
		if (std::holds_alternative<std::string>(p_name)) name = std::get<std::string>(p_name);
		if (std::holds_alternative<bool>(p_pinned)) pinned = std::get<bool>(p_pinned);

		if (type == "role") {
			auto p_role = event.get_parameter("role");
			if (!std::holds_alternative<dpp::snowflake>(p_role)) {
				event.reply(adm_err("need a role for type=role"));
				return;
			}
			r_id = std::get<dpp::snowflake>(p_role);
			if (name.empty()) name = "<@&" + std::to_string(r_id) + ">";
			data = "{}";

		} else if (type == "banner") {
			auto p_file = event.get_parameter("filename");
			if (!std::holds_alternative<std::string>(p_file)) {
				event.reply(adm_err("need a filename for type=banner"));
				return;
			}
			std::string fn = std::get<std::string>(p_file);
			std::string path = "assets/bg/custom/" + fn;
			if (!std::filesystem::exists(path)) {
				event.reply(adm_err("file `" + fn + "` not found in assets/bg/custom/"));
				return;
			}
			if (name.empty()) name = fn;
			data = "{\"file\":\"" + fn + "\"}";

		} else if (type == "xp_boost") {
			auto p_mult = event.get_parameter("multiplier");
			auto p_dur = event.get_parameter("duration");
			double mult = 2.0;
			int hours = 24;
			if (std::holds_alternative<std::string>(p_mult)) {
				try {
					mult = std::stod(std::get<std::string>(p_mult));
				} catch (...) {}
			}
			if (std::holds_alternative<int64_t>(p_dur)) hours = std::get<int64_t>(p_dur);
			if (name.empty()) name = std::to_string((int)mult) + "x xp boost";
			data = "{\"mult\":" + std::to_string(mult) + ",\"hours\":" + std::to_string(hours) + "}";
			if (!std::holds_alternative<bool>(p_pinned)) pinned = true;
		}

		int i_id = db::shop_add(g_id, type, r_id, name, desc, cost, data);
		if (pinned) db::shop_set_int(g_id, i_id, "pinned", 1);
		event.reply(dpp::message("added. id: **" + std::to_string(i_id) + "**").set_flags(dpp::m_ephemeral));
	}

	static void do_remove(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		int id = std::get<int64_t>(event.get_parameter("id"));
		auto p_rel = event.get_parameter("relative");
		bool relative = std::holds_alternative<bool>(p_rel) && std::get<bool>(p_rel);

		if (relative) {
			auto slots = db::bazaar_rotation_get(g_id);
			if (id < 1 || id > (int)slots.size()) {
				event.reply(adm_err("invalid slot"));
				return;
			}
			int item_id = slots[id - 1].item_id;
			db::shop_rmv(g_id, item_id);
			db::bazaar_rotation_clear_slot(g_id, slots[id - 1].slot);
			bazaar::b_refresh_guild(bot, g_id);
		} else {
			db::shop_rmv(g_id, id);
		}
		event.reply(adm_ok());
	}

	bool handle_admin_item_cmd(const std::string& sub, const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		if (sub == "add") {
			do_add(event, bot);
			return true;
		}
		if (sub == "remove") {
			do_remove(event, bot);
			return true;
		}
		if (sub == "toggle") {
			int id = std::get<int64_t>(event.get_parameter("id"));
			db::shop_set_int(g_id, id, "active", db::shop_state(g_id, id, "active") == 1 ? 0 : 1);
			event.reply(adm_ok());
			return true;
		}
		if (sub == "obt") {
			int id = std::get<int64_t>(event.get_parameter("id"));
			db::shop_set_int(g_id, id, "obtainable", db::shop_state(g_id, id, "obtainable") == 1 ? 0 : 1);
			event.reply(adm_ok());
			return true;
		}
		if (sub == "sellable") {
			int id = std::get<int64_t>(event.get_parameter("id"));
			db::shop_set_int(g_id, id, "sellability", db::shop_state(g_id, id, "sellability") == 1 ? 0 : 1);
			event.reply(adm_ok());
			return true;
		}
		if (sub == "price") {
			int id = std::get<int64_t>(event.get_parameter("id"));
			int cost = std::get<int64_t>(event.get_parameter("price"));
			db::shop_set_int(g_id, id, "cost", cost);
			event.reply(adm_ok());
			return true;
		}
		return false;
	}

} // namespace commands
