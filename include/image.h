#pragma once
#include <string>

namespace image {
    std::string img_gen_card(
        const std::string& av_png,
        const std::string& user,
        int level,
        int xp_now,
        int xp_next,
        float progress,
        const std::string& bg_c = "",
        const std::string& credit = "",
        bool invert = false
    );

    std::string img_affect(
        const std::string& img,
        const std::string& effect
    );
}
