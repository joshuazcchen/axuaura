#include "image.h"

#include <ctime>
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

#include "image_constants.h"

static constexpr int INV_BG_W = 1200;
static constexpr int INV_BG_H = 1800;
static constexpr int SLOT_H = 175;

namespace image {

	// declared god knows where i just refactored and idek where i put it
	void draw_inv_placard(Magick::Image& bg, const Magick::Image& tmpl, int px, int py, const db::InvItem& item);

	static constexpr int LEFT_X = 90;
	static constexpr int RIGHT_X = 618;
	static constexpr int START_Y = 330;
	static constexpr int ITEMS_PER_PAGE = 10;
	static constexpr int AV_SIZE = 96;
	static constexpr int TITLE_Y = 255;

	std::string img_gen_inventory(const std::vector<db::InvItem>& items, int page, int total_pages,
								  const std::string& title_name, const std::string& avatar_url) {
		try {
			Magick::Image bg("assets/inventory/inv_bg.png");
			bg.resize(std::to_string(INV_BG_W) + "x" + std::to_string(INV_BG_H) + "!");
			bg.font("assets/fonts/axufont.ttf");
			bg.textAntiAlias(true);
			if (!title_name.empty()) {
				int text_x = LEFT_X;

				if (!avatar_url.empty()) {
					try {
						Magick::Image av(avatar_url);
						av.resize(std::to_string(AV_SIZE) + "x" + std::to_string(AV_SIZE) + "^");
						av.extent(Magick::Geometry(AV_SIZE, AV_SIZE), Magick::CenterGravity);
						av.repage();
						av.alpha(true);
						Magick::Image mask(Magick::Geometry(AV_SIZE, AV_SIZE), Magick::Color("transparent"));
						mask.fillColor("white");
						mask.strokeWidth(0);
						mask.draw(Magick::DrawableCircle(AV_SIZE / 2.0, AV_SIZE / 2.0, AV_SIZE / 2.0 - 0.5, 0));
						av.composite(mask, 0, 0, Magick::DstInCompositeOp);
						bg.composite(av, LEFT_X, TITLE_Y - AV_SIZE + 8, Magick::OverCompositeOp);
						text_x = LEFT_X + AV_SIZE + 12;
					} catch (...) {}
				}

				bg.fontPointsize(72);
				bg.fillColor(Magick::Color("rgba(255,230,160,0.9)"));
				bg.strokeWidth(0);
				bg.draw(Magick::DrawableText(text_x, TITLE_Y, title_name + "'s inventory"));
			}

			std::string dim = std::to_string(PLACARD_W) + "x" + std::to_string(PLACARD_H) + "!";
			Magick::Image tmpl_df("assets/bazaar/placard.png");
			tmpl_df.resize(dim);
			Magick::Image tmpl_xp("assets/bazaar/placard1.png");
			tmpl_xp.resize(dim);
			Magick::Image tmpl_b("assets/bazaar/placard2.png");
			tmpl_b.resize(dim);

			auto get_tmpl = [&](const db::InvItem& item) -> Magick::Image& {
				if (item.type == "xp_boost") return tmpl_xp;
				if (item.type == "banner") return tmpl_b;
				return tmpl_df;
			};

			if (items.empty()) {
				bg.fontPointsize(52);
				bg.fillColor(Magick::Color("rgba(200,180,120,0.90)"));
				bg.draw(Magick::DrawableText(LEFT_X, START_Y + 80, "nothing here yet."));
			}

			for (int i = 0; i < (int)items.size() && i < ITEMS_PER_PAGE; ++i) {
				int col = i % 2;
				int row = i / 2;
				int x = (col == 0) ? LEFT_X : RIGHT_X;
				int y = START_Y + row * SLOT_H;
				if (y + PLACARD_H > INV_BG_H - 30) break;
				draw_inv_placard(bg, get_tmpl(items[i]), x, y, items[i]);
			}

			if (total_pages > 1) {
				std::string pg = std::to_string(page) + " / " + std::to_string(total_pages);
				bg.fontPointsize(32);
				bg.fillColor(Magick::Color("rgba(255,230,160,0.85)"));
				Magick::TypeMetric m;
				bg.fontTypeMetrics(pg, &m);
				bg.draw(Magick::DrawableText((INV_BG_W - m.textWidth()) / 2.0, INV_BG_H - 20, pg));
			}

			Magick::Blob blob;
			bg.write(&blob, "PNG");
			return std::string(static_cast<const char*>(blob.data()), blob.length());
		} catch (Magick::Exception& e) {
			std::cerr << "img_gen_inventory: " << e.what() << "\n";
			return "";
		}
	}

} // namespace image
