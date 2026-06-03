#include <ctime>
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

#include "db.h"

static constexpr int PLACARD_W = 500;
static constexpr int PLACARD_H = 160;
static constexpr int MARGIN_L = 80;
// TODO: make the margins identical since previously it just dealt in single circles and pills
static constexpr int MARGIN_R = 40;
static constexpr int MAX_LEN = 16;

namespace image {

	void draw_inv_placard(Magick::Image& bg, const Magick::Image& tmpl, int px, int py, const db::InvItem& item) {
		bg.composite(tmpl, px, py, Magick::OverCompositeOp);
		bg.strokeWidth(0);

		double cx = px + MARGIN_L;

		std::string name = item.name;
		if (name.size() > (size_t)MAX_LEN) name = name.substr(0, MAX_LEN - 3) + "...";

		bg.fontPointsize(36);
		bg.fillColor("rgba(255,230,160,1)");
		bg.draw(Magick::DrawableText(cx, py + 72, name));

		std::string sub;
		if (item.expires > 0) {
			long secs = item.expires - std::time(nullptr);
			if (secs > 0) {
				sub = secs > 172800 ? "expires ~" + std::to_string(secs / 86400) + "d"
									: "expires ~" + std::to_string(secs / 3600) + "h";
			} else {
				sub = "expired";
			}
		} else if (item.acquired > 0) {
			long days = (std::time(nullptr) - item.acquired) / 86400;
			sub = days == 0 ? "acquired today" : "acquired " + std::to_string(days) + "d ago";
		}

		bg.fontPointsize(30);
		bg.fillColor("rgba(200,180,120,1)");
		if (!sub.empty()) bg.draw(Magick::DrawableText(cx, py + 115, sub));

		if (item.equipped) {
			const std::string eq_txt = "EQUIPPED";
			Magick::TypeMetric em;
			bg.fontPointsize(22);
			bg.fontTypeMetrics(eq_txt, &em);
			bg.fillColor("rgba(140,220,140,1)");
			bg.draw(Magick::DrawableText(px + PLACARD_W - MARGIN_R - em.textWidth(), py + 26, eq_txt));
		}
	}

} // namespace image
