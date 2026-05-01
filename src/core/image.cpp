#include "image.h"
#include <iostream>
#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 1
#include <Magick++.h>

namespace image {

    std::string img_gen_card(const std::string& av_png, const std::string& user, 
                                int level, int xp_now, int xp_next, float progress) {
        try {
            std::string bg_file = "assets/bg/lvltmp.png";
            if (level >= 60) bg_file = "assets/bg/lvltmp.png";
            else if (level >= 40) bg_file = "assets/bg/lvltmp.png";
            else if (level >= 20) bg_file = "assets/bg/lvltmp.png";
            else if (level >= 10) bg_file = "assets/bg/lvltmp.png";

            Magick::Image bg(bg_file);
            bg.resize("900x280!");

            Magick::Blob av_blob(av_png.data(), av_png.length());
            Magick::Image avatar(av_blob);
            avatar.resize("128x128!");

            Magick::Image mask("128x128", "none");
            mask.fillColor("black");
            mask.draw(Magick::DrawableCircle(64, 64, 64, 128));
            
            avatar.composite(mask, 0, 0, Magick::DstInCompositeOp);

            bg.composite(avatar, 40, 76, Magick::OverCompositeOp);

            bg.font("assets/fonts/axufont.ttf");
            bg.textAntiAlias(true);
            
            bg.fillColor("white");
            bg.strokeColor("black");
            bg.strokeWidth(1);
            bg.fontPointsize(48);
            bg.draw(Magick::DrawableText(200, 110, user));

            bg.fontPointsize(32);
            bg.draw(Magick::DrawableText(200, 150, "Level " + std::to_string(level)));

            std::string xp_text = std::to_string(xp_now) + " / " + std::to_string(xp_next) + " XP";
            bg.draw(Magick::DrawableText(700, 150, xp_text));

            double bar_x1 = 200, bar_y1 = 170;
            double bar_x2 = 850, bar_y2 = 200;
            double radius = 15;

            bg.strokeWidth(0);
            bg.fillColor(Magick::Color("rgba(0, 0, 0, 0.6)"));
            bg.draw(Magick::DrawableRoundRectangle(bar_x1, bar_y1, bar_x2, bar_y2, radius, radius));

            if (progress > 0.0f) {
                double fill_x2 = bar_x1 + ((bar_x2 - bar_x1) * progress);
                if (fill_x2 < bar_x1 + radius*2) fill_x2 = bar_x1 + radius*2; 
                
                bg.fillColor(Magick::Color("#71F28F")); 
                bg.draw(Magick::DrawableRoundRectangle(bar_x1, bar_y1, fill_x2, bar_y2, radius, radius));
            }

            Magick::Blob output;
            bg.magick("PNG");
            bg.write(&output);
            return std::string(static_cast<const char*>(output.data()), output.length());

        } catch (Magick::Exception& error) {
            std::cerr << "image gen error: " << error.what() << std::endl;
            return "";
        }
    }
}
