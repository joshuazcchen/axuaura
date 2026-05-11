#pragma once
#include <dpp/dpp.h>

namespace xp {
struct UserXP {
    int xp;
    int level;
    int xp_next;
    int xp_this;
    float progress;
};

int xp_req(int level);

UserXP xp_getxp(dpp::snowflake guild_id, dpp::snowflake user_id);

int award(dpp::cluster& bot, dpp::snowflake guild_id, dpp::snowflake user_id, dpp::snowflake channel_id,
    dpp::snowflake announce_channel);
} // namespace xp
