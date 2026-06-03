#pragma once
#include <string>
#include <vector>

#include "db.h"

namespace image {
	std::string img_gen_card(const std::string& av_png, const std::string& user, int level, int xp_now, int xp_next,
							 float progress, const std::string& bg_c = "", const std::string& credit = "",
							 bool invert = false, const std::vector<std::string>& badges = {});

	std::vector<std::string> img_gen_bazaar(const std::vector<db::ShopItem>& positive_items,
											const std::vector<db::ShopItem>& negative_items, long next_restock = 0);

	struct BoardEntry {
		std::string display;
		std::string avatar_url;
		int value;
		int secondary;
	};

	std::string img_gen_leaderboard(const std::vector<BoardEntry>& entries);
	std::string img_gen_auraboard(const std::vector<BoardEntry>& entries, bool is_bottom, int total_aura);
	std::string img_gen_inventory(const std::vector<db::InvItem>& items, int page, int pages,
								  const std::string& title_name = "", const std::string& avatar_url = "");
} // namespace image
