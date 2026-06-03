#include "image.h"

#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

namespace image {

	static constexpr int BRD_W = 1200;
	static constexpr int BRD_H = 1800;

	struct PodiumSpec {
		int cx;
		int top_y;
		int font_large;
		int font_small;
	};
	static constexpr PodiumSpec PODIUMS[3] = {
		{200, 400, 46, 36},
		{600, 350, 56, 40},
		{1000, 450, 40, 32},
	};
	static constexpr int PODIUM_BOTTOM = 570;

	static constexpr int ROW_CENTERS[7] = {681, 848, 1010, 1173, 1336, 1499, 1672};
	static constexpr int ROW_RANK_X = 55;
	static constexpr int ROW_NAME_X = 265;
	static constexpr int ROW_VAL_X = 1155;

	static void draw_avatar_circle(Magick::Image& img, const std::string& url, int cx, int cy, int size) {
		if (url.empty()) return;
		try {
			Magick::Image av(url);
			av.resize(std::to_string(size) + "x" + std::to_string(size) + "^");
			av.extent(Magick::Geometry(size, size), Magick::CenterGravity);
			av.repage();
			av.alpha(true);
			Magick::Image mask(Magick::Geometry(size, size), Magick::Color("transparent"));
			mask.fillColor("white");
			mask.strokeWidth(0);
			mask.draw(Magick::DrawableCircle(size / 2.0, size / 2.0, size / 2.0 - 0.5, 0));
			av.composite(mask, 0, 0, Magick::DstInCompositeOp);
			img.composite(av, cx - size / 2, cy - size / 2, Magick::OverCompositeOp);
		} catch (...) {}
	}

	static void draw_centered(Magick::Image& img, const std::string& txt, int cx, int y, int ptsize,
			const std::string& color) {
		img.fontPointsize(ptsize);
		img.fillColor(Magick::Color(color));
		img.strokeWidth(0);
		Magick::TypeMetric tm;
		img.fontTypeMetrics(txt, &tm);
		img.draw(Magick::DrawableText(cx - tm.textWidth() / 2.0, y, txt));
	}

	static void draw_podium_entry(Magick::Image& img, int rank_idx, const BoardEntry& e, bool show_level) {
		const PodiumSpec& p = PODIUMS[rank_idx];
		int avail = PODIUM_BOTTOM - p.top_y;
		int line1_y = p.top_y + avail * 30 / 100;
		int line2_y = p.top_y + avail * 58 / 100;
		int line3_y = p.top_y + avail * 84 / 100;

		draw_avatar_circle(img, e.avatar_url, p.cx, line2_y - 60, 52);

		static const char* medals[] = {"#2", "#1", "#3"};
		draw_centered(img, medals[rank_idx], p.cx, line1_y, p.font_large, "rgba(56,53,55,0.95)");
		std::string name = e.display;
		if (name.size() > 14) name = name.substr(0, 12) + "\xe2\x80\xa6";
		draw_centered(img, name, p.cx, line2_y, p.font_small, "rgba(56,53,55,0.90)");

		std::string val = show_level && e.secondary > 0
			? "lv." + std::to_string(e.secondary) + " / " + std::to_string(e.value) + "xp"
			: std::to_string(e.value);
		draw_centered(img, val, p.cx, line3_y, p.font_small - 4, "rgba(80,60,40,0.90)");
	}

	static void draw_row_entry(Magick::Image& img, int rank, const BoardEntry& e, int cy, bool show_level) {
		img.strokeWidth(0);
		draw_avatar_circle(img, e.avatar_url, ROW_NAME_X - 20, cy + 2, 40);

		img.fontPointsize(52);
		img.fillColor(Magick::Color("rgba(255,255,255,0.90)"));
		img.draw(Magick::DrawableText(ROW_RANK_X, cy + 20, "#" + std::to_string(rank)));

		std::string name = e.display;
		if (name.size() > 20) name = name.substr(0, 18) + "\xe2\x80\xa6";
		img.fontPointsize(48);
		img.fillColor(Magick::Color("rgba(240,235,225,0.95)"));
		img.draw(Magick::DrawableText(ROW_NAME_X, cy + 20, name));

		std::string val = show_level && e.secondary > 0
			? "lv." + std::to_string(e.secondary) + "  " + std::to_string(e.value) + "xp"
			: std::to_string(e.value);
		img.fontPointsize(40);
		img.fillColor(Magick::Color("rgba(200,220,255,0.90)"));
		Magick::TypeMetric tm;
		img.fontTypeMetrics(val, &tm);
		img.draw(Magick::DrawableText(ROW_VAL_X - tm.textWidth(), cy + 18, val));
	}

	static std::string render_board(const std::string& bg_file, const std::string& title,
			const std::vector<BoardEntry>& entries, bool show_level) {
		Magick::Image bg(bg_file);
		bg.resize(std::to_string(BRD_W) + "x" + std::to_string(BRD_H) + "!");
		bg.font("assets/fonts/axufont.ttf");
		bg.textAntiAlias(true);

		bg.fontPointsize(72);
		bg.fillColor(Magick::Color("rgba(255,255,255,0.9)"));
		bg.strokeWidth(0);
		Magick::TypeMetric tm;
		bg.fontTypeMetrics(title, &tm);
		bg.draw(Magick::DrawableText((BRD_W - tm.textWidth()) / 2.0, 300, title));

		static const int podium_rank_order[] = {1, 0, 2};
		for (int i = 0; i < (int)entries.size() && i < 3; ++i) {
			int slot = (i == 0) ? 1 : (i == 1) ? 0 : 2;
			draw_podium_entry(bg, slot, entries[i], show_level);
		}

		for (int i = 3; i < (int)entries.size() && (i - 3) < 7; ++i) {
			draw_row_entry(bg, i + 1, entries[i], ROW_CENTERS[i - 3], show_level);
		}

		Magick::Blob out;
		bg.magick("PNG");
		bg.write(&out);
		return std::string(static_cast<const char*>(out.data()), out.length());
	}

	std::string img_gen_leaderboard(const std::vector<BoardEntry>& entries) {
		try {
			return render_board("assets/leaderboards/good_board.png", "LEADERBOARD", entries, true);
		} catch (Magick::Exception& e) {
			std::cerr << "img_gen_leaderboard: " << e.what() << "\n";
			return "";
		}
	}

	std::string img_gen_auraboard(const std::vector<BoardEntry>& entries, bool is_bottom, int total_aura) {
		try {
			std::string file = is_bottom ? "assets/leaderboards/evil_board.png" : "assets/leaderboards/good_board.png";
			std::string title = is_bottom ? "AURALESS" : "AURAFUL";
			std::string img = render_board(file, title, entries, false);
			return img;
		} catch (Magick::Exception& e) {
			std::cerr << "img_gen_auraboard: " << e.what() << "\n";
			return "";
		}
	}

} // namespace image
