#include "message.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <random>
#include <regex>

#include "config.h"
#include "db.h"
#include "xp.h"

namespace message {
std::string resp_msg(const std::vector<std::string>& pool)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, pool.size() - 1);
    return pool[dis(gen)];
}

void handle(const dpp::message_create_t& event, dpp::cluster& bot)
{
    // TODO: make proper helpers and consolidate this code properly.
    if (event.msg.author.is_bot())
        return;
    dpp::snowflake g_id = event.msg.guild_id;
    auto conf = config::get_config(g_id);

    static std::random_device rd;
    static std::mt19937 gen(rd());

    dpp::snowflake user_id = event.msg.author.id;

    long time_now = std::time(nullptr);
    long time_prev = db::xp_time_get(event.msg.guild_id, user_id);

    if (time_now - time_prev >= conf.xp_cooldown) {
        std::uniform_int_distribution<> xp_dis(conf.xp_min, conf.xp_max);

        int xp_del = xp_dis(gen);
        db::xp_add(event.msg.guild_id, user_id, xp_del);
        db::xp_time_set(event.msg.guild_id, user_id, time_now);
        int xp_now = db::xp_get(event.msg.guild_id, user_id);
        int lvl_now = db::lvl_get(event.msg.guild_id, user_id);
        int xp_requ = xp::xp_req(lvl_now + 1);

        if (xp_now >= xp_requ) {
            int lvl_new = lvl_now + 1;
            db::xp_lvl_set(event.msg.guild_id, user_id, xp_now, lvl_new);
            bot.message_create(dpp::message(conf.lvl_ch, "<@" + std::to_string(user_id) + "> reached **level " + std::to_string(lvl_new) + "**, nice."));
            if (conf.lvl_roles.find(lvl_new) != conf.lvl_roles.end()) {
                bot.guild_member_add_role(event.msg.guild_id, user_id, conf.lvl_roles.at(lvl_new));
            }
        }
    }

    auto& allowed = conf.allowed_channels;
    if (std::find(allowed.begin(), allowed.end(), event.msg.channel_id) == allowed.end())
        return;

    std::uniform_int_distribution<> dis(1, 100);
    auto is_special = std::find(conf.specials.begin(), conf.specials.end(), user_id);
    if (is_special != conf.specials.end() && dis(gen) == 1) {
        db::rmv_aura(event.msg.guild_id, user_id, 100);
        bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" HOLY AURA LOSS 💔"));
        return;
    }

    if (dis(gen) <= conf.aurachancegain) {
        db::add_aura(event.msg.guild_id, user_id, conf.aurapassiveamt);
    }

    std::uniform_int_distribution<> drop_dis(1, 1000);
    if (drop_dis(gen) == 1) {
        int u_aura = db::get_aura(event.msg.guild_id, user_id);
        int sign = (u_aura >= 0) ? 1 : -1;
        auto items = db::shop_get_sign(event.msg.guild_id, sign);

        if (!items.empty()) {
            auto weight_get = [](int cost) -> double {
                double cost_scaled = std::abs(cost) / 1000.0;
                return 1.0 / std::pow(cost_scaled + 1.0, 2.0);
            };

            double total_weight = 0;
            for (auto& item : items)
                total_weight += weight_get(item.cost);

            std::uniform_real_distribution<> weight_dis(0, total_weight);
            double roll = weight_dis(gen);
            int picked_id = items.front().item_id;
            db::ShopItem* picked = &items.front();

            for (auto& item : items) {
                roll -= weight_get(item.cost);
                if (roll <= 0) {
                    picked_id = item.item_id;
                    picked = &item;
                    break;
                }
            }

            if (db::inv_has(event.msg.guild_id, user_id, picked_id)) {
                int refund = picked->cost / 10;
                db::add_aura(event.msg.guild_id, user_id, refund);
                bot.message_create(
                    dpp::message(event.msg.channel_id, "<@" + std::to_string(user_id) + "> found **" + picked->name + "** but they already owned it so its been converted to " + std::to_string(refund) + "** aura instead"));
            } else {
                db::inv_add(event.msg.guild_id, user_id, picked_id);
                double weight = weight_get(picked->cost);
                double pool = total_weight / weight;
                long long odds = static_cast<long long>(pool * 1000.0);

                bot.message_create(dpp::message(
                    event.msg.channel_id, "<@" + std::to_string(user_id) + "> is lucky as hell and stumbled across a **" + picked->name + "** (1/" + std::to_string(odds) + ") ! access with `/inventory view`."));
            }
        }
    }

    std::regex url_r(R"(https?://[^\s]+)");
    std::smatch match;
    std::string content = event.msg.content;

    if (std::regex_search(content, match, url_r)) {
        std::string url = match.str();
        bool is_reliable = false;
        dpp::channel* ch = dpp::find_channel(event.msg.channel_id);
        if (!ch)
            return; // i dont know if we'll ever actually need this check but honestly it was there in the original
                    // copy so i dont wanna risk it
        auto perm = ch->get_user_permissions(&(event.msg.author));
        for (const auto& p : config::RELIABLE_PROVIDERS)
            if (url.find(p) != std::string::npos)
                is_reliable = true;
        if (!is_reliable || content.find("<" + url + ">") != std::string::npos)
            return;
        for (const auto& attachment : event.msg.attachments) {
            if (attachment.filename.size() >= 4 && (attachment.filename.substr(attachment.filename.size() - 4) == ".gif" || attachment.filename.substr(attachment.filename.size() - 4) == ".png" || attachment.filename.substr(attachment.filename.size() - 4) == ".jpg")) {
                if (ch) {
                    db::rmv_aura(event.msg.guild_id, event.msg.author.id,
                        db::get_setting_int(event.msg.guild_id, "auralossamt", 100));
                    bot.channel_typing(event.msg.channel_id);
                    std::string msg = (event.msg.channel_id == conf.non_eng_ch) ? resp_msg(config::SPANISH_LOSS)
                                                                                : resp_msg(config::AURA_LOSSES);
                    dpp::message rep(event.msg.channel_id, msg);
                    rep.set_reference(event.msg.id);
                    bot.message_create(rep);
                    return;
                }
            }
        }
        if (ch) {
            dpp::snowflake msg_id = event.msg.id;
            dpp::snowflake ch_id = event.msg.channel_id;
            dpp::snowflake user_sf = event.msg.author.id;
            dpp::snowflake g_id = event.msg.guild_id;
            if (!(perm & dpp::p_embed_links)) {
                bot.channel_typing(event.msg.channel_id);
                bot.start_timer(
                    [&bot, msg_id, ch_id, user_sf, g_id, conf](dpp::timer t) {
                        db::rmv_aura(g_id, user_sf, db::get_setting_int(g_id, "auralossamt", 100));

                        bot.channel_typing(ch_id);
                        std::string msg = (ch_id == conf.non_eng_ch) ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);
                        dpp::message rep(ch_id, msg);
                        rep.set_reference(msg_id);
                        bot.message_create(rep);
                        bot.stop_timer(t);
                    },
                    3);
                return;
            } else {
                bot.start_timer(
                    [&bot, msg_id, ch_id, url, user_sf, g_id, conf](dpp::timer t) {
                        bot.message_get(
                            msg_id, ch_id,
                            [&bot, ch_id, msg_id, url, user_sf, g_id, conf](const dpp::confirmation_callback_t& res) {
                                if (res.is_error())
                                    return;
                                dpp::message m = std::get<dpp::message>(res.value);
                                if (m.embeds.empty() && m.content.find("<" + url + ">") == std::string::npos) {
                                    db::rmv_aura(g_id, user_sf, db::get_setting_int(g_id, "auralossamt", 100));

                                    std::string msg = (ch_id == conf.non_eng_ch) ? resp_msg(config::SPANISH_LOSS)
                                                                                 : resp_msg(config::AURA_LOSSES);
                                    dpp::message rep(ch_id, msg);
                                    rep.set_reference(msg_id);
                                    bot.message_create(rep);
                                }
                            });
                        bot.stop_timer(t);
                    },
                    5);
            }
        }
    }
}
} // namespace message
