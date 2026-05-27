#pragma once
#include <string>
#include <vector>

#include "db.h"

namespace image {
	std::string img_gen_card(const std::string& av_png, const std::string& user, int level, int xp_now, int xp_next,
							 float progress, const std::string& bg_c = "", const std::string& credit = "",
							 bool invert = false);

	std::string img_gen_bazaar(const std::vector<db::ShopItem>& positive_items,
							   const std::vector<db::ShopItem>& negative_items
							   long next_restock = 0);
} // namespace image
