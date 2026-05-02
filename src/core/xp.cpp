#include "xp.h"
#include "db.h"
#include <random>
#include <ctime>
#include <nlohmann/json.hpp>

namespace xp {

    int xp_req(int level) {
	
		if (level == -1) return std::numeric_limits<int>::max();
        int xp = 0;

        double xp_base = 75.0;
        double xp_mult = 1.10;
        double xp_cap = 7500.0;

        for (int i = 1; i < level; ++i) {
            double xp_del = xp_base * std::pow(xp_mult, i - 1);
            xp += (int)std::min(xp_del, xp_cap);
        }

        return xp;
    }

    UserXP xp_getxp(dpp::snowflake guild_id, dpp::snowflake user_id) {
        UserXP stat;
        stat.xp = db::xp_get(guild_id, user_id);
        stat.level = db::lvl_get(guild_id, user_id);
        
        int current_floor = xp_req(stat.level);
        int next_req = xp_req(stat.level + 1);
        
        stat.xp_next = next_req - stat.xp;
        stat.xp_this = stat.xp - current_floor;
        
        int level_total_diff = next_req - current_floor;
        if (level_total_diff > 0) {
            stat.progress = static_cast<float>(stat.xp_this) / level_total_diff;
        } else {
            stat.progress = 0.0f;
        }

        return stat;
    }

    int award(dpp::cluster& bot, dpp::snowflake guild_id, dpp::snowflake user_id, 
              dpp::snowflake channel_id, dpp::snowflake announce_channel) {
        
        long time_now = std::time(nullptr);
        long time_prev = db::xp_time_get(guild_id, user_id);
        int cooldown = db::get_setting_int(guild_id, "xp_cooldown", 60);

        if (time_now - time_prev < cooldown) {
            return -1; 
        }

        int xp_min = db::get_setting_int(guild_id, "xp_min", 15);
        int xp_max = db::get_setting_int(guild_id, "xp_max", 30);
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(xp_min, xp_max);
        double xp_mult = db::inv_xp_mult(guild_id, user_id);
        int xp_del = (int)(dis(gen) * xp_mult);

        db::xp_add(guild_id, user_id, xp_del);
        db::xp_time_set(guild_id, user_id, time_now);

        int xp_now = db::xp_get(guild_id, user_id);
        int lvl_now = db::lvl_get(guild_id, user_id);
        int lvl_new = lvl_now;

        bool leveled_up = false;
        while (xp_now >= xp_req(lvl_new + 1)) {
            lvl_new++;
            leveled_up = true;
        }

        if (leveled_up) {
            db::xp_lvl_set(guild_id, user_id, xp_now, lvl_new);

            dpp::snowflake target_channel = announce_channel ? announce_channel : channel_id;
            bot.message_create(dpp::message(target_channel, 
                "<@" + std::to_string(user_id) + "> reached **level " + std::to_string(lvl_new) + "**, nice."));

            std::string roles_json_str = db::get_setting_str(guild_id, "xp_level_roles", "{}");
            try {
                nlohmann::json roles_map = nlohmann::json::parse(roles_json_str);
                std::string lvl_key = std::to_string(lvl_new);
                
                if (roles_map.contains(lvl_key)) {
                    std::string role_str = roles_map[lvl_key].get<std::string>();
                    dpp::snowflake role_id = std::stoull(role_str);
                    
                    bot.guild_member_add_role(guild_id, user_id, role_id);
                }
            } catch (const nlohmann::json::exception& e) {
                bot.log(dpp::ll_error, "failed parse of xp " + 
                                       std::to_string(guild_id) + ": " + e.what());
            }

            return lvl_new;
        }

        return -1;
    }
}
