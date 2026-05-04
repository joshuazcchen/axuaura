#include "db.h"
#include "config.h"
#include "roles.h"
#include <dpp/dpp.h>

namespace roles {
    void role_sync(dpp::cluster &bot) {
        dpp::snowflake g_id = 1469591363770122274;
        auto conf = config::get_config(g_id);

        // TODO: migrate this accordingly
        // right now its just using my old hard coded id.
        auto top3 = db::get_ab(g_id, 3, false);
        auto bot3 = db::get_ab(g_id, 3, true);

        std::map<dpp::snowflake, dpp::snowflake> targets;
        int leader_id = db::shop_ensure_sys(g_id, "most aura", conf.leader_role, 50000);
        int loser_id = db::shop_ensure_sys(g_id, "least aura", conf.loser_role, -50000);
        int bot3_id = db::shop_ensure_sys(g_id, "3nd least", conf.bot3_role, -50000);
        int bot2_id = db::shop_ensure_sys(g_id, "2nd least", conf.bot2_role, -50000);
        int num2_id = db::shop_ensure_sys(g_id, "2nd most", conf.num2_role, 50000);
        int num3_id = db::shop_ensure_sys(g_id, "3nd most", conf.num3_role, 50000);

        auto do_rank = [&](const std::vector<std::pair<std::string, int>>& list, bool bot) {
            for (int i = 0; i < list.size(); i++) {
                int rank = i+1;
                dpp::snowflake u_id(list[i].first);
                int role_id = 0;

                if (bot) {
                    if (rank == 3) role_id = num3_id;
                    else if (rank == 2) role_id = num2_id;
                    else if (rank == 1) role_id = leader_id;
                } else {
                    if (rank == 3) role_id = bot3_id;
                    else if (rank == 2) role_id = bot2_id;
                    else if (rank == 1) role_id = loser_id;
                }
                targets[u_id] = role_id;
            }
        };
        do_rank(top3, false);
        do_rank(bot3, true);

        // TODO: make this not depend on randomly huge numbers.
        // TODO: make this modular
        
        // TODO: make this work with the other roles i deadass looked at this yesterday and somehow forgot that there were 6 people with special aura roles.
        for (auto const& [u_id, r_id] : targets) {
            db::inv_add(g_id, u_id, r_id);
            db::inv_eq(g_id, u_id, r_id);
        }

        for (auto const& r_id : conf.stupid_roles) {
            dpp::role *r = dpp::find_role(r_id);
            auto hold = r->get_members();
            for (auto const& [u_id, memb] : hold) {
                bool is_stupid = (targets.count(u_id) && targets.at(u_id) == r_id);

                if (!is_stupid) {
                    bot.guild_member_remove_role(1469591363770122274, u_id, r_id);
                    if (r_id == conf.leader_role) db::inv_rm(g_id, u_id, leader_id);
                    if (r_id == conf.loser_role) db::inv_rm(g_id, u_id, loser_id);
                    if (r_id == conf.bot2_role) db::inv_rm(g_id, u_id, bot2_id);
                    if (r_id == conf.bot3_role) db::inv_rm(g_id, u_id, bot3_id);
                    if (r_id == conf.num2_role) db::inv_rm(g_id, u_id, num2_id);
                    if (r_id == conf.num3_role) db::inv_rm(g_id, u_id, num3_id);
                }
            }
        };
    }
}
