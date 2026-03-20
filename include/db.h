#pragma once
#include <dpp/dpp.h>
#include <string>

namespace db {
    void init();
    int get_aura(dpp::snowflake user_id);

    void set_aura(dpp::snowflake user_id, int amount);
    void add_aura(dpp::snowflake user_id, int amount);
    void rmv_aura(dpp::snowflake user_id, int amount);
    std::vector<std::pair<std::string, int>> get_ab(bool bottom);

    int get_total_aura();
}