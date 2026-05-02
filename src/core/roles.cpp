#include "db.h"
#include "config.h"
#include "roles.h"
#include <dpp/dpp.h>

namespace roles {
    void role_sync(dpp::cluster &bot) {
        dpp::snowflake g_id = 1469591363770122274;

        // TODO: migrate this accordingly
        // right now its just using my old hard coded id.
        auto top3 = db::get_ab(g_id, 3, false);
        auto bot3 = db::get_ab(g_id, 3, true);

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

        // TODO: make this not depend on randomly huge numbers.
        int leader_id = db::shop_ensure_sys(g_id, "Leader Role", config::LEADER, 50000000);
        int loser_id = db::shop_ensure_sys(g_id, "Loser Role", config::LOSER, -50000000);
        
        for (auto const& [u_id, r_id] : targets) {
            // TODO: fix the guild id stuff
            bot.guild_member_add_role(1469591363770122274, u_id, r_id);
            if (r_id == config::LEADER && !db::inv_has(g_id, u_id, leader_id)) {
                db::inv_add(g_id, u_id, leader_id);
                db::inv_eq(g_id, u_id, leader_id);
            } else if (r_id == config::LOSER && !db::inv_has(g_id, u_id, loser_id)) {
                db::inv_add(g_id, u_id, loser_id);
                db::inv_eq(g_id, u_id, loser_id);
            }
        }

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
                    if (r_id == config::LEADER) db::inv_rm(g_id, u_id, leader_id);
                    if (r_id == config::LOSER) db::inv_rm(g_id, u_id, loser_id);
                }
            }
        };
    }
}
