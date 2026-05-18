#include <algorithm>
#include <ctime>
#include <random>

#include "bazaar.h"
#include "db.h"

namespace bazaar {

	void b_refresh_guild(dpp::cluster& bot, dpp::snowflake g_id) {
		int rotating_cnt = db::get_setting_int(g_id, "bazaar_rotating_cnt", 2);
		int refresh_hours = db::get_setting_int(g_id, "bazaar_refresh_hours", 168);
		long now = std::time(nullptr);
		long stale_before = now - (long)refresh_hours * 3600;

		auto pinned = db::shop_get_pinned(g_id);
		for (size_t i = 0; i < pinned.size(); ++i)
			db::bazaar_rotation_set(g_id, (int)i, pinned[i].item_id);

		auto pool = db::shop_get_rotating_pool(g_id);
		auto current = db::bazaar_rotation_get(g_id);

		std::vector<int> chosen;
		for (auto& s : current)
			chosen.push_back(s.item_id);

		static std::mt19937 rng(std::random_device{}());

		int base_slot = (int)pinned.size();
		for (int s = 0; s < rotating_cnt; ++s) {
			int slot = base_slot + s;

			bool slot_fresh = false;
			for (auto& cs : current) {
				if (cs.slot == slot && cs.refreshed_at > stale_before) {
					slot_fresh = true;
					break;
				}
			}
			if (slot_fresh) continue;

			std::vector<db::ShopItem> candidates;
			for (auto& p : pool) {
				if (std::find(chosen.begin(), chosen.end(), p.item_id) == chosen.end()) candidates.push_back(p);
			}
			if (candidates.empty()) break;

			std::shuffle(candidates.begin(), candidates.end(), rng);
			db::bazaar_rotation_set(g_id, slot, candidates[0].item_id);
			chosen.push_back(candidates[0].item_id);
		}

		for (auto& cs : current) {
			if (cs.slot >= base_slot + rotating_cnt) db::bazaar_rotation_clear_slot(g_id, cs.slot);
		}

		b_post_ui(bot, g_id);
	}

	void b_refresh_all(dpp::cluster& bot) {
		auto guilds = db::settings_get_guilds_with("bazaar_channel");
		for (auto g : guilds)
			b_refresh_guild(bot, g);
	}

} // namespace bazaar
