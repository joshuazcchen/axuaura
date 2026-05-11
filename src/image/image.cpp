#include "image.h"
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

namespace image {

	std::string img_gen_card(const std::string& av_png, const std::string& user, 
			int level, int xp_now, int xp_next, float progress, const std::string& bg_c, const std::string& artist, bool invert) {
		try {
			std::string bg_file;
			if (!bg_c.empty()) {
				bg_file = "assets/bg/custom/" + bg_c;
			} else {
				if (level >= 60) bg_file = "assets/bg/60_tmp.png";
				else if (level >= 40) bg_file = "assets/bg/40_tmp.png";
				else if (level >= 20) bg_file = "assets/bg/20_tmp.png";
				else if (level >= 10) bg_file = "assets/bg/10_tmp.png";
				else bg_file = "assets/bg/0_tmp.png"; 
			}

			Magick::Image bg(bg_file);
			bg.resize("1800x560!");

			double img_left = 100;

			Magick::Blob av_blob(av_png.data(), av_png.length());
			Magick::Image avatar(av_blob);
			avatar.resize("256x256!");

			avatar.backgroundColor("none");
			avatar.alphaChannel(Magick::SetAlphaChannel);

			Magick::Image mask("256x256", "none");
			mask.fillColor("white");
			mask.draw(Magick::DrawableCircle(128, 128, 128, 256));

			// avatar
			avatar.composite(mask, 0, 0, Magick::DstInCompositeOp);
			bg.composite(avatar, img_left, 100, Magick::OverCompositeOp);
			if (invert) {
				bg.strokeAntiAlias(true);
				bg.strokeColor("white");
				bg.strokeWidth(12);
				bg.fillColor("transparent");
				bg.draw(Magick::DrawableCircle(img_left + 128, 100 + 128, img_left + 128, 100));
			}

			// username and other text
			bg.font("assets/fonts/axufont.ttf");
			bg.textAntiAlias(true);
			std::string base = "rgba(56, 53, 55, 0.8)";

			double img_text_x = img_left + 300;
			double img_bar_x = 1170;

			// name
			std::string display_name = "@" + user;
			bg.fontPointsize(100);
			Magick::TypeMetric metrics;
			bg.fontTypeMetrics(display_name, &metrics);
			double width = metrics.textWidth();

			if (invert) {
				bg.strokeColor("white");
				bg.strokeWidth(16);
				bg.fillColor("white");
				bg.draw(Magick::DrawableText(img_text_x, 200, display_name));
			}
			bg.strokeWidth(0);
			bg.fillColor(Magick::Color(base));
			bg.draw(Magick::DrawableText(img_text_x, 200, display_name));

			// divider
			bg.strokeLineCap(Magick::RoundCap);
			if (invert) {
				bg.strokeColor("white");
				bg.strokeWidth(24);
				bg.draw(Magick::DrawableLine(img_text_x, 230, img_text_x + width + 15, 230));
			}
			bg.strokeColor(Magick::Color(base));
			bg.strokeWidth(16);
			bg.draw(Magick::DrawableLine(img_text_x, 230, img_text_x + width + 15, 230));

			// stats
			bg.fontPointsize(64);
			std::string stats_text = "LEVEL " + (level != -1 ? std::to_string(level)  : "-") + "   " + (level != -1 ? std::to_string(xp_now) : "-") + " / " + (level != -1 ? std::to_string(xp_next) : "-") + " XP";
			if (invert) {
				bg.strokeColor("white");
				bg.strokeWidth(16);
				bg.fillColor("white");
				bg.draw(Magick::DrawableText(img_text_x, 320, stats_text));
			}
			bg.strokeWidth(0);
			bg.fillColor(Magick::Color(base));
			bg.draw(Magick::DrawableText(img_text_x, 320, stats_text));

			double bar_y1 = 390;
			double bar_y2 = 450;
			double radius = 20;

			bg.strokeColor(invert ? "white" : "transparent");
			bg.strokeWidth(invert ? 8 : 0);
			bg.fillColor(Magick::Color(base));
			bg.draw(Magick::DrawableRoundRectangle(img_left, bar_y1, img_bar_x, bar_y2, radius, radius));
			bg.strokeWidth(0);

			if (level == -1) {
				progress = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
			}

			if (progress > 0.0f) {
				double fill_x2 = img_left + ((img_bar_x - img_left) * progress);
				if (fill_x2 < img_left + radius*2) fill_x2 = img_left + radius*2; 

				bg.fillColor(Magick::Color("#99DAFB")); 
				bg.draw(Magick::DrawableRoundRectangle(img_left, bar_y1, fill_x2, bar_y2, radius, radius));
			}

			bg.fontPointsize(60);
			if (!artist.empty()) {
				if (invert) {
					bg.strokeColor("white");
					bg.strokeWidth(6);
					bg.fillColor("white");
					bg.annotate(artist, Magick::Geometry("+10+10"), Magick::SouthEastGravity);
				}
				bg.strokeWidth(0);
				bg.fillColor(Magick::Color(base));
				bg.annotate(artist, Magick::Geometry("+10+10"), Magick::SouthEastGravity);
			} else if (bg_c.empty()) {
				if (invert) {
					bg.strokeColor("white");
					bg.strokeWidth(6);
					bg.fillColor("white");
					bg.annotate("axuaxi", Magick::Geometry("+10+10"), Magick::SouthEastGravity);
				}
				bg.strokeWidth(0);
				bg.fillColor(Magick::Color(base));
				bg.annotate("axuaxi", Magick::Geometry("+10+10"), Magick::SouthEastGravity);
			}

			Magick::Blob output;
			bg.magick("PNG");
			bg.write(&output);
			return std::string(static_cast<const char*>(output.data()), output.length());

		} catch (Magick::Exception& error) {
			std::cerr << "image gen error: " << error.what() << std::endl;
			return "";
		} catch (...) {
			std::cerr << "big bad issue!" << std::endl;
			return "";
		}
	}
}
