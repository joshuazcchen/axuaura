#include <filesystem>

#include "bazaar.h"
#include "commands.h"
#include "config.h"
#include "db.h"
#include "utils.h"

namespace commands {

	static dpp::message adm_ok() { return dpp::message("done.").set_flags(dpp::m_ephemeral); }
	static dpp::message adm_err(const std::string& s) { return dpp::message(s).set_flags(dpp::m_ephemeral); }

	static std::string build_role_data(const dpp::slashcommand_t& ev) {
		std::string btn = "primary", c1, c2;
		auto pb = ev.get_parameter("button_colour");
		if (std::holds_alternative<std::string>(pb)) btn = std::get<std::string>(pb);
		std::string d = "{\"button_style\":\"" + btn + "\"";
		d += "}";
		return d;
	}

	static std::string build_banner_data(const dpp::slashcommand_t& ev, const std::string& fn) {
		std::string artist;
		bool invert = false;
		bool is_global = false;
		auto pa = ev.get_parameter("artist");
		auto pi = ev.get_parameter("invert");
		auto pg = ev.get_parameter("global");
		if (std::holds_alternative<std::string>(pa)) artist = std::get<std::string>(pa);
		if (std::holds_alternative<bool>(pi)) invert = std::get<bool>(pi);
		if (std::holds_alternative<bool>(pg)) is_global = std::get<bool>(pg);
		return "{\"file\":\"" + fn +
			   "\","
			   "\"artist\":\"" +
			   artist +
			   "\","
			   "\"invert\":" +
			   (invert ? "true" : "false") +
			   ","
			   "\"global\":" +
			   (is_global ? "true" : "false") + "}";
	}

	static void do_add(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		std::string type = std::get<std::string>(event.get_parameter("type"));
		int cost = std::get<int64_t>(event.get_parameter("cost"));
		std::string name, desc, data;
		dpp::snowflake r_id = 0;
		bool pinned = false;

		auto pname = event.get_parameter("name");
		auto pdesc = event.get_parameter("desc");
		auto ppinned = event.get_parameter("pinned");
		if (std::holds_alternative<std::string>(pname)) name = std::get<std::string>(pname);
		if (std::holds_alternative<std::string>(pdesc)) desc = std::get<std::string>(pdesc);
		if (std::holds_alternative<bool>(ppinned)) pinned = std::get<bool>(ppinned);

		if (type == "role") {
			auto pr = event.get_parameter("role");
			if (!std::holds_alternative<dpp::snowflake>(pr)) {
				event.reply(dpp::message("need a role for type=role").set_flags(dpp::m_ephemeral));
				return;
			}
			r_id = std::get<dpp::snowflake>(pr);
			if (name.empty()) name = "<&" + std::to_string(r_id) + ">";
		} else if (type == "banner") {
			auto pf = event.get_parameter("filename");
			if (!std::holds_alternative<std::string>(pf)) {
				event.reply(adm_err("need a filename for type=banner"));
				return;
			}
			std::string fn = std::get<std::string>(pf);
			if (!std::filesystem::exists("assets/bg/bazaar/" + fn)) {
				event.reply(adm_err("file `" + fn + "` not found in assets/bg/bazaar/"));
				return;
			}
			if (name.empty()) name = fn;
			data = build_banner_data(event, fn);
			bool is_global = utils::json_bool(data, "global");
			int i_id = db::shop_add(g_id, type, r_id, name, desc, cost, data);
			if (pinned) db::shop_set_int(g_id, i_id, "pinned", 1);
			if (is_global) db::shop_set_int(g_id, i_id, "global", 1);
			event.reply(dpp::message("added banner id **" + std::to_string(i_id) + "**").set_flags(dpp::m_ephemeral));
			return;

		} else if (type == "xp_boost") {
			auto pm = event.get_parameter("multiplier");
			auto pd = event.get_parameter("duration");
			double mult = 2.0;
			int hours = 24;
			if (std::holds_alternative<std::string>(pm)) try {
					mult = std::stod(std::get<std::string>(pm));
				} catch (...) {}
			if (std::holds_alternative<int64_t>(pd)) hours = (int)std::get<int64_t>(pd);
			if (name.empty()) name = std::to_string((int)mult) + "x XP Boost";
			data = "{\"mult\":" + std::to_string(mult) + ",\"hours\":" + std::to_string(hours) + "}";
			if (!std::holds_alternative<bool>(ppinned)) pinned = true;
		}

		std::string btn = "primary";
		auto pb = event.get_parameter("button_colour");
		if (std::holds_alternative<std::string>(pb)) btn = std::get<std::string>(pb);
		data = "{\"button_style\":\"" + btn + "\"}";
		int i_id = db::shop_add(g_id, type, r_id, name, desc, cost, data);
		if (pinned) db::shop_set_int(g_id, i_id, "pinned", 1);
		event.reply(dpp::message("added id **" + std::to_string(i_id) + "**").set_flags(dpp::m_ephemeral));
	}

	static void do_remove(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		int id = std::get<int64_t>(event.get_parameter("id"));
		auto pr = event.get_parameter("relative");
		bool relative = std::holds_alternative<bool>(pr) && std::get<bool>(pr);

		int compensation = 0;
		auto pc = event.get_parameter("compensation");
		if (std::holds_alternative<int64_t>(pc)) compensation = (int)std::get<int64_t>(pc);

		int item_id = id;
		if (relative) {
			auto slots = db::bazaar_rotation_get(g_id);
			if (id < 1 || id > (int)slots.size()) {
				event.reply(adm_err("invalid slot"));
				return;
			}
			item_id = slots[id - 1].item_id;
		}

		auto item = db::shop_get(g_id, item_id);
		if (item.item_id == -1) {
			event.reply(adm_err("item not found"));
			return;
		}

		auto owners = db::shop_delete(g_id, item_id, compensation);
		bazaar::b_refresh_guild(bot, g_id);

		std::string reply = "deleted **" + item.name + "**";
		if (!owners.empty()) {
			reply += " and removed it from " + std::to_string(owners.size()) + " inventor" +
					 (owners.size() == 1 ? "y" : "ies");
			if (compensation > 0) reply += " (+" + std::to_string(compensation) + " aura compensation each)";
		}
		event.reply(dpp::message(reply).set_flags(dpp::m_ephemeral));
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
