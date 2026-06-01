#include <algorithm>
#include <ctime>
#include <dppp/dppp.h>
#include <random>

#include "bazaar.h"
#include "db.h"

// ranges:
// 0..9 positive pinned items
// 10..99 positive rotating items
// 100..109 negative pinned items
// 110..199 negative rotating items

namespace bazaar {

	static void b_fill_rotating(dpp::snowflake g_id, int base_slot, int count, std::vector<db::ShopItem>& pool,
								const std::vector<db::RotationSlot>& current, std::vector<int>& chosen,
								long stale_before) {
		thread_local std::philox4x32 rng(std::random_device{}());

		for (int s = 0; s < count; ++s) {
			int slot = base_slot + s;
			bool fresh = false;
			for (auto& cs : current)
				if (cs.slot == slot && cs.refreshed_at > stale_before) {
					fresh = true;
					break;
				}
			if (fresh) continue;

			std::vector<db::ShopItem> cands;
			for (auto& p : pool)
				if (std::find(chosen.begin(), chosen.end(), p.item_id) == chosen.end()) cands.push_back(p);
			if (cands.empty()) {
				int cur_item = -1;
				for (auto& cs : current) {
					if (cs.slot == slot) {
						cur_item = cs.item_id;
						break;
					}
				}
				for (auto& p : pool) {
					if (p.item_id != cur_item) {
						cands.push_back(p);
					}
				}
				if (cands.empty()) {
					if (cur_item != -1) db::bazaar_rotation_set(g_id, slot, cur_item);
					continue;
				}
			}

			std::shuffle(cands.begin(), cands.end(), rng);
			db::bazaar_rotation_set(g_id, slot, cands[0].item_id);
			chosen.push_back(cands[0].item_id);
		}

		for (auto& cs : current)
			if (cs.slot >= base_slot && cs.slot < base_slot + 90 && cs.slot >= base_slot + count)
				db::bazaar_rotation_clear_slot(g_id, cs.slot);
	}

	void b_refresh_guild(dpp::cluster& bot, dpp::snowflake g_id) {
		int pos_cnt = db::get_setting_int(g_id, "bazaar_positive_cnt", 2);
		int neg_cnt = db::get_setting_int(g_id, "bazaar_negative_cnt", 2);
		long refresh_s = (long)db::get_setting_int(g_id, "bazaar_refresh_hours", 168) * 3600;
		long now = std::time(nullptr);
		long stale_before = now - refresh_s;

		auto pinned = db::shop_get_pinned(g_id);
		int pp = 0, np = 0;
		for (auto& item : pinned) {
			if (item.cost >= 0)
				db::bazaar_rotation_set(g_id, pp++, item.item_id);
			else
				db::bazaar_rotation_set(g_id, 100 + np++, item.item_id);
		}
		auto current = db::bazaar_rotation_get(g_id);
		for (auto& cs : current) {
			if (cs.slot >= 0 && cs.slot < 10 && cs.slot >= pp) db::bazaar_rotation_clear_slot(g_id, cs.slot);
			if (cs.slot >= 100 && cs.slot < 110 && cs.slot >= 100 + np) db::bazaar_rotation_clear_slot(g_id, cs.slot);
		}

		auto pool = db::shop_get_rotating_pool(g_id);
		std::vector<db::ShopItem> pos_pool, neg_pool;
		for (auto& p : pool)
			(p.cost >= 0 ? pos_pool : neg_pool).push_back(p);

		current = db::bazaar_rotation_get(g_id);
		std::vector<int> chosen;
		for (auto& s : current)
			chosen.push_back(s.item_id);

		b_fill_rotating(g_id, 10, pos_cnt, pos_pool, current, chosen, stale_before);
		b_fill_rotating(g_id, 110, neg_cnt, neg_pool, current, chosen, stale_before);

		b_post_ui(bot, g_id);
	}

	void b_refresh_all(dpp::cluster& bot) {
		for (auto g : db::settings_get_guilds_with("bazaar_channel"))
			b_refresh_guild(bot, g);
	}

} // namespace bazaar
