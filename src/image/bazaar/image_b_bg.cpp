// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "image.h"

#include <ctime>
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

#include "db.h"

namespace image {

	void draw_placard(Magick::Image& bg, const Magick::Image& tmpl, int px, int py, const db::ShopItem& item);

	static constexpr int BG_W = 1245;
	static constexpr int BG_H = 1400;
	static constexpr int LEFT_PANEL_X = 75;
	static constexpr int RIGHT_PANEL_X = 668;
	static constexpr int PANEL_W = 500;
	static constexpr int PLACARD_W = 500;
	static constexpr int PLACARD_H = 160;
	static constexpr int PANEL_Y = 500;
	static constexpr int SLOT_H = 170;
	static constexpr int PAGE_SIZE = 5;
	static constexpr int RESTOCK_Y = 390;

	static std::string render_page(const std::vector<db::ShopItem>& pos_slice,
								   const std::vector<db::ShopItem>& neg_slice, long next_restock, bool is_last_page,
								   int page, int total_pages) {
		Magick::Image bg("assets/bazaar/bazaar_bg.png");
		bg.resize(std::to_string(BG_W) + "x" + std::to_string(BG_H) + "!");
		bg.font("assets/fonts/axufont.ttf");
		bg.textAntiAlias(true);

		std::string dim = std::to_string(PLACARD_W) + "x" + std::to_string(PLACARD_H) + "!";

		bool need_xp = false, need_banner = false;
		for (const auto& item : pos_slice) {
			if (item.type == "xp_boost")
				need_xp = true;
			else if (item.type == "banner")
				need_banner = true;
		}
		for (const auto& item : neg_slice) {
			if (item.type == "xp_boost")
				need_xp = true;
			else if (item.type == "banner")
				need_banner = true;
		}

		Magick::Image tmpl_df("assets/bazaar/placard.png");
		tmpl_df.resize(dim);
		Magick::Image tmpl_xp, tmpl_b;
		if (need_xp) {
			tmpl_xp.read("assets/bazaar/placard1.png");
			tmpl_xp.resize(dim);
		}
		if (need_banner) {
			tmpl_b.read("assets/bazaar/placard2.png");
			tmpl_b.resize(dim);
		}

		auto get_tmpl = [&](const db::ShopItem& item) -> Magick::Image& {
			if (item.type == "xp_boost" && need_xp) return tmpl_xp;
			if (item.type == "banner" && need_banner) return tmpl_b;
			return tmpl_df;
		};

		int margin = (PANEL_W - PLACARD_W) / 2;
		int left_x = LEFT_PANEL_X + margin;
		int right_x = RIGHT_PANEL_X + margin;
		int max_y = BG_H - PLACARD_H - 30;

		for (size_t i = 0; i < pos_slice.size(); ++i) {
			int y = PANEL_Y + (int)i * SLOT_H;
			if (y > max_y) break;
			draw_placard(bg, get_tmpl(pos_slice[i]), left_x, y, pos_slice[i]);
		}
		for (size_t i = 0; i < neg_slice.size(); ++i) {
			int y = PANEL_Y + (int)i * SLOT_H;
			if (y > max_y) break;
			draw_placard(bg, get_tmpl(neg_slice[i]), right_x, y, neg_slice[i]);
		}

		if (total_pages > 1) {
			std::string pg = "" + std::to_string(page) + " / " + std::to_string(total_pages);
			bg.fontPointsize(30);
			bg.fillColor("rgba(255,255,255,0.85)");
			Magick::TypeMetric m;
			bg.fontTypeMetrics(pg, &m);
			bg.draw(Magick::DrawableText((BG_W - m.textWidth()) - 15, BG_H - 15, pg));
		}

		if (next_restock > 0) {
			std::time_t now = std::time(nullptr);
			std::tm* tm_info = std::localtime(&now);
			char date_l[64];
			std::strftime(date_l, sizeof(date_l), "%b %d", tm_info);

			tm_info = std::localtime(&next_restock);
			char date_r[64];
			std::strftime(date_r, sizeof(date_r), "%b %d, %H:%M", tm_info);

			double lxc = LEFT_PANEL_X + (PANEL_W / 2.0);
			double rxc = RIGHT_PANEL_X + (PANEL_W / 2.0);

			int l1y = RESTOCK_Y;
			int l2y = RESTOCK_Y + 28;

			std::string colour_t = "rgba(255,230,160,1)";
			std::string colour_b = "rgba(255,200,100,1)";

			std::vector<Magick::Drawable> text_l;
			text_l.push_back(Magick::DrawableTextAlignment(Magick::CenterAlign));
			text_l.push_back(Magick::DrawablePointSize(32));

			text_l.push_back(Magick::DrawableFillColor(colour_t));
			text_l.push_back(Magick::DrawableText(lxc, l1y, "Shop for"));
			text_l.push_back(Magick::DrawableFillColor(colour_b));
			text_l.push_back(Magick::DrawableText(lxc, l2y, date_l));

			bg.draw(text_l);

			std::vector<Magick::Drawable> text_r;
			text_r.push_back(Magick::DrawableTextAlignment(Magick::CenterAlign));
			text_r.push_back(Magick::DrawablePointSize(32));

			text_r.push_back(Magick::DrawableFillColor(colour_t));
			text_r.push_back(Magick::DrawableText(rxc, l1y, "Restocks on:"));
			text_r.push_back(Magick::DrawableFillColor(colour_b));
			text_r.push_back(Magick::DrawableText(rxc, l2y, date_r));

			bg.draw(text_r);
		}

		Magick::Blob blob;
		bg.write(&blob, "PNG");
		return std::string(static_cast<const char*>(blob.data()), blob.length());
	}

	std::vector<std::string> img_gen_bazaar(const std::vector<db::ShopItem>& pos, const std::vector<db::ShopItem>& neg,
											long next_restock) {
		std::vector<std::string> pages;
		int total = std::max({(int)pos.size(), (int)neg.size(), 1});
		int num_pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;

		for (int p = 0; p < num_pages; ++p) {
			int start = p * PAGE_SIZE;
			auto pos_slice = std::vector<db::ShopItem>(pos.begin() + std::min(start, (int)pos.size()),
													   pos.begin() + std::min(start + PAGE_SIZE, (int)pos.size()));
			auto neg_slice = std::vector<db::ShopItem>(neg.begin() + std::min(start, (int)neg.size()),
													   neg.begin() + std::min(start + PAGE_SIZE, (int)neg.size()));

			pages.push_back(render_page(pos_slice, neg_slice, next_restock, p == num_pages - 1, p + 1, num_pages));
		}
		return pages;
	}

} // namespace image
