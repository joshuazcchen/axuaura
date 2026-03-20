#pragma once
#include <dpp/dpp.h>
#include <vector>
#include <string>

namespace config {
    extern std::vector<dpp::snowflake> SPECIALS;
    extern std::vector<dpp::snowflake> ALLOWED_CHANNELS;

    extern const std::vector<std::string> AURA_LOSSES;
    extern const std::vector<std::string> SPANISH_LOSS;
    extern const std::vector<std::string> RELIABLE_PROVIDERS;
}