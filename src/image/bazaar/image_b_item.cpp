#include <filesystem>
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>
#include <string>

#include "db.h"
#include "utils.h"

static constexpr int PLACARD_W = 500;
static constexpr int PLACARD_H = 160;
static constexpr int TEXT_AREA_W = 360;
static constexpr int IND_X = 370;
static constexpr int IND_W = 80;
static constexpr int IND_H = 80;
static constexpr int PILL_H = 40;
static constexpr int PILL_W = 80;
static constexpr int MARGIN_L = 80;
static constexpr int MARGIN_R = 15;
static constexpr int MAX_LEN = 16;
static constexpr int BDR_SZ = 4;

namespace image {

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
						std::string c_name = cached_role->name;
						std::erase_if(c_name, [](unsigned char c) { return !(std::isalnum(c) || c == ' '); });
						return c_name.size() <= MAX_LEN ? c_name : c_name.substr(0, MAX_LEN - 3) + "...";
					}
				} catch (const std::exception&) {}
			}
			return "Role";
		}

		return item.name.size() <= MAX_LEN ? item.name : item.name.substr(0, MAX_LEN - 3) + "...";
	}

	static std::string cost_label(const db::ShopItem& item) {
		std::string s = std::to_string(std::abs(item.cost)) + " aura";
		if (item.cost < 0) s = "-" + s;
		if (item.pinned) s += " *";
		return s;
	}

	static void preview_banner(Magick::Image& bg, int px, int py, const std::string& data) {
		std::string fn = utils::json_str(data, "file");
		if (fn.empty()) return;
		std::string ipath = "assets/bg/bazaar/" + fn;
		if (!std::filesystem::exists(ipath)) return;
		try {
			Magick::Image preview(ipath);

			std::string geom_str = std::to_string(PILL_W) + "x" + std::to_string(PILL_H) + "^";
			preview.resize(geom_str);
			preview.extent(Magick::Geometry(PILL_W, PILL_H), Magick::CenterGravity);
			preview.repage();
			preview.alpha(true);

			Magick::Image mask(Magick::Geometry(PILL_W, PILL_H), Magick::Color("transparent"));
			mask.fillColor("white");
			mask.strokeColor("transparent");
			mask.strokeWidth(0);

			mask.draw(Magick::DrawableRoundRectangle(0, 0, PILL_W - 1, PILL_H - 1, PILL_H / 2.0, PILL_H / 2.0));

			preview.composite(mask, 0, 0, Magick::DstInCompositeOp);

			int total_w = PILL_W + (BDR_SZ * 2);
			int total_h = PILL_H + (BDR_SZ * 2);

			Magick::Image f_pill(Magick::Geometry(total_w, total_h), Magick::Color("transparent"));
			f_pill.fillColor(Magick::Color("#3B3637"));
			f_pill.strokeColor("transparent");
			f_pill.strokeWidth(0);

			f_pill.draw(Magick::DrawableRoundRectangle(0, 0, total_w - 1, total_h - 1, total_h / 2.0, total_h / 2.0));

			f_pill.composite(preview, BDR_SZ, BDR_SZ, Magick::OverCompositeOp);

			int ix = px + IND_X - MARGIN_R;
			int iy = py + (PLACARD_H - PILL_H) / 2;

			bg.composite(f_pill, ix - BDR_SZ, iy - BDR_SZ, Magick::OverCompositeOp);
		} catch (...) {}
	}

	static void preview_xp(Magick::Image& bg, int px, int py) {
		const char* ipath = "assets/icons/xp_boost.png";
		if (!std::filesystem::exists(ipath)) return;
		try {
			Magick::Image preview(ipath);
			preview.resize("72x72>");
			int ix = px + IND_X + (IND_W - (int)preview.columns()) / 2;
			int iy = py + (PLACARD_H - (int)preview.rows()) / 2;
			bg.composite(preview, ix, iy, Magick::OverCompositeOp);
		} catch (...) {}
	}

	static void preview_role(Magick::Image& bg, int px, int py, const std::string& data) {
		std::string c1 = utils::json_str(data, "colour1");
		if (c1.empty()) return;
		std::string c2 = utils::json_str(data, "colour2");
		int gx = px + IND_X - MARGIN_R;
		int gy = py + (PLACARD_H - 40 - BDR_SZ) / 2;
		try {
			Magick::Image grad;
			if (!c2.empty()) {
				grad.size(Magick::Geometry(PILL_W * 2, PILL_H));
				grad.read("radial-gradient:" + c1 + "-" + c2);
				grad.crop(Magick::Geometry(PILL_W, PILL_H, 0, 0));
			} else {
				grad = Magick::Image(Magick::Geometry(PILL_W, PILL_H), Magick::Color(c1));
			}
			grad.repage();
			grad.alpha(true);

			Magick::Image mask(Magick::Geometry(PILL_W, PILL_H), Magick::Color("transparent"));
			mask.fillColor("white");
			mask.strokeColor("transparent");
			mask.strokeWidth(0);
			mask.draw(Magick::DrawableRoundRectangle(0, 0, PILL_W - 1, PILL_H - 1, PILL_H / 2.0, PILL_H / 2.0));

			grad.composite(mask, 0, 0, Magick::DstInCompositeOp);

			int total_w = PILL_W + (BDR_SZ * 2);
			int total_h = PILL_H + (BDR_SZ * 2);

			Magick::Image f_pill(Magick::Geometry(total_w, total_h), Magick::Color("transparent"));

			f_pill.fillColor(Magick::Color("#3B3637"));
			f_pill.strokeWidth(0);
			f_pill.draw(Magick::DrawableRoundRectangle(0, 0, total_w - 1, total_h - 1, total_h / 2.0, total_h / 2.0));

			f_pill.composite(grad, BDR_SZ, BDR_SZ, Magick::OverCompositeOp);

			bg.composite(f_pill, gx - BDR_SZ, gy - BDR_SZ, Magick::OverCompositeOp);
		} catch (...) {}
	}

	void draw_placard(Magick::Image& bg, const Magick::Image& tmpl, int px, int py, const db::ShopItem& item) {
		bg.composite(tmpl, px, py, Magick::OverCompositeOp);
		if (item.type == "banner")
			preview_banner(bg, px, py, item.data);
		else if (item.type == "xp_boost")
			preview_xp(bg, px, py);
		else if (item.type == "role")
			preview_role(bg, px, py, item.data);

		bool has_ind = (item.type == "banner" || item.type == "xp_boost" || item.type == "role");
		double cx = px + MARGIN_L;

		Magick::TypeMetric m;
		std::string name = safe_name(item);
		std::string cost = cost_label(item);

		bg.strokeWidth(0);
		bg.fontPointsize(36);
		bg.fillColor("rgba(255,230,160,1)");
		bg.fontTypeMetrics(name, &m);
		bg.draw(Magick::DrawableText(cx, py + 72, name));

		bg.fontPointsize(30);
		bg.fillColor("rgba(255,200,100,1)");
		bg.fontTypeMetrics(cost, &m);
		bg.draw(Magick::DrawableText(cx, py + 115, cost));
	}

} // namespace image
