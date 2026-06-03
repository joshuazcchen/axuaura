#include <algorithm>
#include <atomic>
#include <malloc.h>

#include "commands.h"
#include "db.h"
#include "image.h"

namespace commands {

	static std::atomic_bool ab_rendering{false};

	static std::string get_display_name(dpp::snowflake guild_id, dpp::snowflake user_id) {
		dpp::guild* g = dpp::find_guild(guild_id);
		if (g) {
			auto mit = g->members.find(user_id);
			if (mit != g->members.end()) {
				if (!mit->second.get_nickname().empty()) return mit->second.get_nickname();
			}
		}
		dpp::user* u = dpp::find_user(user_id);
		if (u && !u->username.empty()) return u->username;
		return std::to_string(user_id);
	}

	static bool is_guild_member(dpp::snowflake guild_id, dpp::snowflake user_id) {
		dpp::guild* g = dpp::find_guild(guild_id);
		if (!g) return true;
		return g->members.find(user_id) != g->members.end();
	}

	dpp::slashcommand auraboard_def(dpp::cluster& bot) {
		return dpp::slashcommand("auraboard", "who has lost the most aura?", bot.me.id)
			.add_option(dpp::command_option(dpp::co_string, "sort", "top or bottom 10", false)
					.add_choice(dpp::command_option_choice("top", std::string("top")))
					.add_choice(dpp::command_option_choice("bottom", std::string("bottom"))))
			.add_option(dpp::command_option(dpp::co_user, "who", "check a specific user", false));
	}

	static void aura_reply_single(const dpp::slashcommand_t& event, dpp::snowflake g_id,
			dpp::snowflake target, dpp::snowflake caller) {
		int aura = db::get_aura(g_id, target);
		int rank = db::aura_rank(g_id, target);
		bool self = (target == caller);
		std::string who = self ? "You have" : "<@" + std::to_string(target) + "> has";
		std::string body = who + " **" + std::to_string(aura) + "** aura (rank #" + std::to_string(rank) + ")";
		event.reply(
				dpp::message(body).set_flags(dpp::m_ephemeral).set_allowed_mentions(false, false, false, false, {}, {}));
	}

	void handle_auraboard(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake caller = event.command.get_issuing_user().id;

		auto who_param = event.get_parameter("who");
		if (std::holds_alternative<dpp::snowflake>(who_param)) {
			aura_reply_single(event, g_id, std::get<dpp::snowflake>(who_param), caller);
			return;
		}

		auto sort_param = event.get_parameter("sort");
		std::string sort_mode =
			std::holds_alternative<std::string>(sort_param) ? std::get<std::string>(sort_param) : "me";

		if (sort_mode == "me") {
			aura_reply_single(event, g_id, caller, caller);
			return;
		}

		event.thinking();
		bool is_bottom = (sort_mode == "bottom");

		auto raw = db::get_ab(g_id, 50, is_bottom);
		std::vector<image::BoardEntry> entries;
		entries.reserve(15);
		for (auto& [uid_str, aura] : raw) {
			dpp::snowflake uid;
			try { uid = std::stoull(uid_str); } catch (...) { continue; }
			if (!is_guild_member(g_id, uid)) continue;
			entries.push_back({get_display_name(g_id, uid), aura, 0});
			if ((int)entries.size() >= 15) break;
		}

		if (entries.empty()) {
			event.edit_original_response(dpp::message("auraboard is empty."));
			return;
		}

		while (ab_rendering.exchange(true)) {}
		std::string img;
		try { img = image::img_gen_auraboard(entries, is_bottom, db::get_total_aura(g_id)); } catch (...) {}
		ab_rendering = false;
		malloc_trim(0);

		if (img.empty()) {
			event.edit_original_response(dpp::message("couldn't generate auraboard image."));
			return;
		}

		dpp::message msg(event.command.channel_id, "");
		msg.add_file("auraboard.png", img);
		event.edit_original_response(msg);
	}

} // namespace commands
