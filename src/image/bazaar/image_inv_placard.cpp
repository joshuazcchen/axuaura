#include <cstdio>
#include <ctime>
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

#include "db.h"
#include "image_constants.h"
#include "utils.h"

static constexpr int MARGIN_R = 70;
static constexpr int MARGIN_T = 70;

namespace image {

	void preview_banner(Magick::Image& bg, int px, int py, const std::string& data);
	void preview_xp(Magick::Image& bg, int px, int py);
	void preview_role(Magick::Image& bg, int px, int py, const std::string& data);

	void draw_inv_placard(Magick::Image& bg, const Magick::Image& tmpl, int px, int py, const db::InvItem& item) {
		bg.composite(tmpl, px, py, Magick::OverCompositeOp);
		bg.strokeWidth(0);

		double cx = px + MARGIN_L;

		std::string name = utils::get_safe_role(item.name, item.type, 18);

		bg.fontPointsize(36);
		bg.fillColor("rgba(255,230,160,1)");
		bg.draw(Magick::DrawableText(cx, py + 72, name));

		std::string sub;
		Magick::Color sub_colour("rgba(200,180,120,1)");
		if (item.expires > 0) {
			long secs = item.expires - std::time(nullptr);
			if (secs > 0) {
				sub = secs > 172800 ? "expires ~" + std::to_string(secs / 86400) + "d"
									: "expires ~" + std::to_string(secs / 3600) + "h";
			} else {
				sub = "expired";
				sub_colour = Magick::Color("rgba(220,100,100,1)");
			}
		} else if (item.equipped) {
			sub = "equipped";
			sub_colour = Magick::Color("rgba(140,220,140,1)");
		} else if (item.acquired > 0) {
			long days = (std::time(nullptr) - item.acquired) / 86400;
			sub = days == 0 ? "acquired today" : "acquired " + std::to_string(days) + "d ago";
		}

		bg.fontPointsize(30);
		bg.fillColor(sub_colour);
		if (!sub.empty()) bg.draw(Magick::DrawableText(cx, py + 115, sub));

		if (item.type == "banner") {
			preview_banner(bg, px, py, item.data);
		} else if (item.type == "xp_boost") {
			preview_xp(bg, px, py);
		} else if (item.type == "role") {
			std::string rdata = item.data;
			if (utils::json_str(rdata, "colour1").empty() && item.role_id) {
				if (dpp::role* r = dpp::find_role(item.role_id); r && r->colour) {
					char hex[8];
					std::snprintf(hex, sizeof(hex), "#%06x", r->colour & 0xFFFFFF);
					rdata = "{\"colour1\":\"" + std::string(hex) + "\"}";
				}
			}
			preview_role(bg, px, py, item.data);
		}
	}

} // namespace image
