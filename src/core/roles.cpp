#include "db.h"
#include "config.h"
#include "roles.h"
#include <dpp/dpp.h>

namespace roles {
    void role_sync(dpp::cluster &bot) {
        dpp::snowflake g_id = 1469591363770122274;

        auto top3 = db::get_ab(3, false);
        auto bot3 = db::get_ab(3, true);

        std::map<dpp::snowflake, dpp::snowflake> targets;

        auto do_rank = [&](const std::vector<std::pair<std::string, int>>& list, bool bot) {
            for (int i = 0; i < list.size(); i++) {
                int rank = i+1;
                dpp::snowflake u_id(list[i].first);
                dpp::snowflake role = 0;

                if (bot) {
                    if (rank == 3) role = config::BOT3;
                    else if (rank == 2) role = config::BOT2;
                    else if (rank == 1) role = config::LOSER;
                } else {
                    if (rank == 3) role = config::NUM3;
                    else if (rank == 2) role = config::NUM2;
                    else if (rank == 1) role = config::LEADER;
                }
                targets[u_id] = role;
            }
        };
        do_rank(top3, false);
        do_rank(bot3, true);
        
        for (auto const& r_id : config::STUPID_ROLES) {
            dpp::role *r = dpp::find_role(r_id);
            if (!r) {
                std::cout<<"role die"<<std::endl;
                continue;
            }
            auto hold = r->get_members();
            for (auto const& [u_id, memb] : hold) {
                bool is_stupid = (targets.count(u_id) && targets.at(u_id) == r_id);

                if (!is_stupid) {
                    bot.guild_member_remove_role(1469591363770122274, u_id, r_id);
                }
            }
        };

        for (auto const& [u_id, r_id] : targets) {
            bot.guild_member_add_role(1469591363770122274, u_id, r_id);
        }
    }
}
