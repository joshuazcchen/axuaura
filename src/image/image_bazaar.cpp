#include "image.h"

#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

namespace image {

	static constexpr int BG_W = 1245;
	static constexpr int BG_H = 1400;
	static constexpr int LEFT_PANEL_X = 75;
	static constexpr int RIGHT_PANEL_X = 668;
	static constexpr int PANEL_W = 500;
	static constexpr int PLACARD_W = 500;
	static constexpr int PLACARD_H = 160;
	static constexpr int PANEL_Y = 500;
	static constexpr int SLOT_H = 170;

	static std::string safe_name(const db::ShopItem& item) {
		if (item.type == "role" && !item.name.empty() && item.name[0] == '<') {
			std::string id_str;
			for (char c : item.name) {
				if (std::isdigit(c)) { id_str += c; }
			}

			if (!id_str.empty()) {
				try {
					dpp::snowflake role_id = std::stoull(id_str);
					dpp::role* cached_role = dpp::find_role(role_id);
					if (cached_role) {
						return cached_role->name.size() <= 22 ? cached_role->name
															  : cached_role->name.substr(0, 20) + "..";
					}
				} catch (const std::exception&) {}
			}
			return "Role";
		}

		return item.name.size() <= 22 ? item.name : item.name.substr(0, 20) + "..";
	}

	static std::string cost_label(const db::ShopItem& item) {
		std::string s = std::to_string(std::abs(item.cost)) + " aura";
		if (item.cost < 0) s = "-" + s;
		if (item.pinned) s += " [always]";
		return s;
	}

	static void draw_placard(Magick::Image& bg, const Magick::Image& tmpl, int px, int py, const db::ShopItem& item) {
		bg.composite(tmpl, px, py, Magick::OverCompositeOp);

		Magick::TypeMetric m;

		std::string name = safe_name(item);
		bg.fontPointsize(36);
		bg.strokeWidth(0);
		bg.fillColor("rgba(255,230,160,1)");
		bg.fontTypeMetrics(name, &m);
		double nx = px + (PLACARD_W - m.textWidth()) / 2.0;
		bg.draw(Magick::DrawableText(nx, py + 62, name));

		std::string cost = cost_label(item);
		bg.fontPointsize(30);
		bg.fillColor("rgba(255,200,100,1)");
		bg.fontTypeMetrics(cost, &m);
		double cx = px + (PLACARD_W - m.textWidth()) / 2.0;
		bg.draw(Magick::DrawableText(cx, py + 105, cost));
	}

	std::string img_gen_bazaar(const std::vector<db::ShopItem>& pos_items, const std::vector<db::ShopItem>& neg_items) {
		try {
			Magick::Image bg("assets/bazaar/bazaar_bg.png");
			bg.resize(std::to_string(BG_W) + "x" + std::to_string(BG_H) + "!");
			bg.font("assets/fonts/axufont.ttf");
			bg.textAntiAlias(true);

			Magick::Image tmpl("assets/bazaar/placard.png");
			tmpl.resize(std::to_string(PLACARD_W) + "x" + std::to_string(PLACARD_H) + "!");

			int margin = (PANEL_W - PLACARD_W) / 2;
			int left_x = LEFT_PANEL_X + margin;
			int right_x = RIGHT_PANEL_X + margin;
			int max_y = BG_H - PLACARD_H - 20;

			for (size_t i = 0; i < pos_items.size(); ++i) {
				int y = PANEL_Y + (int)i * SLOT_H;
				if (y > max_y) break;
				draw_placard(bg, tmpl, left_x, y, pos_items[i]);
			}
			for (size_t i = 0; i < neg_items.size(); ++i) {
				int y = PANEL_Y + (int)i * SLOT_H;
				if (y > max_y) break;
				draw_placard(bg, tmpl, right_x, y, neg_items[i]);
			}

			Magick::Blob blob;
			bg.write(&blob, "PNG");
			return std::string(static_cast<const char*>(blob.data()), blob.length());

		} catch (const std::exception& e) {
			std::cerr << e.what() << "\n";
			return "";
		}
	}

} // namespace image
