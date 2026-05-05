#include "commands.h"
#include "db.h"
#include "config.h"
#include <algorithm>
#include "utils.h"

namespace commands {

    dpp::slashcommand bazaar_def(dpp::cluster& bot) {
        return dpp::slashcommand("bazaar", "IT'S HERE!", bot.me.id)
            .add_option(dpp::command_option(dpp::co_sub_command, "list", "browse the wares")
                .add_option(dpp::command_option(dpp::co_integer, "page", "page number", true))
            )
            .add_option(dpp::command_option(dpp::co_sub_command, "buy", "buy item")
                .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
            )
            .add_option(dpp::command_option(dpp::co_sub_command, "sell", "sell item")
                .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
            )
            .add_option(dpp::command_option(dpp::co_sub_command_group, "admin", "ADMIN COMMANDS")
                .add_option(dpp::command_option(dpp::co_sub_command, "add", "add role")
                    .add_option(dpp::command_option(dpp::co_role, "role", "the role", true))
                    .add_option(dpp::command_option(dpp::co_integer, "cost", "cost", true))
                    .add_option(dpp::command_option(dpp::co_string, "desc", "desc", false))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "remove", "rmv")
                    .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "listall", "list")
                    .add_option(dpp::command_option(dpp::co_integer, "page", "page number", true))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "toggle", "toggle item")
                    .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "obt", "toggle item obtain")
                    .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "price", "price item")
                    .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
                    .add_option(dpp::command_option(dpp::co_integer, "price", "item id", true))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "give", "give item to a user")
                    .add_option(dpp::command_option(dpp::co_user, "user", "user id", true))
                    .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
                )
                .add_option(dpp::command_option(dpp::co_sub_command, "take", "like taking candy from an axuaxi")
                    .add_option(dpp::command_option(dpp::co_user, "user", "user id", true))
                    .add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
                )
            );
    }

    void handle_bazaar(const dpp::slashcommand_t& event, dpp::cluster& bot) {
        auto cmd = event.command.get_command_interaction();
        std::string sub = cmd.options[0].name;
        dpp::snowflake g_id = event.command.guild_id;
        dpp::snowflake u_id = event.command.get_issuing_user().id;

        if (sub == "list") {
            int page = std::get<int64_t>(event.get_parameter("page"));
            page--;
            auto items_full = db::shop_get_all(g_id, true);
            size_t start_idx = static_cast<size_t>(page) * config::BAZAAR_PGSZ;
            if (start_idx > items_full.size()) {
                start_idx = items_full.size();
            }
            size_t count = std::min(static_cast<size_t>(config::BAZAAR_PGSZ), items_full.size() - start_idx);

            auto items = std::span{items_full}.subspan(start_idx, count);
            if (items.empty()) {
                event.reply("no");
                return;
            }
            std::string out = "# the bazaar\n\n";
            for (auto& i : items) {
                std::string display = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
                out += "**id: " + std::to_string(i.item_id) + "**, " + display + ", **price (tax inclusive): ** " + std::to_string(i.cost) + " aura\n";
                if (!i.desc.empty()) out += "> *" + i.desc + "*\n";
            }
            out += "\n-# page " + std::to_string(++page) + " of " + std::to_string((items_full.size() + config::BAZAAR_PGSZ) / config::BAZAAR_PGSZ);
            event.reply(dpp::message(out).set_allowed_mentions(false, false, false, false, {}, {}));

        } else if (sub == "buy") {
            int id = std::get<int64_t>(event.get_parameter("id"));
            auto item = db::shop_get(g_id, id);

            if (item.item_id == -1 || !item.active) {
                event.reply(dpp::message("what?").set_flags(dpp::m_ephemeral));
                return;
            }
            if (db::inv_has(g_id, u_id, id)) {
                event.reply(dpp::message("you already own this").set_flags(dpp::m_ephemeral));
                return;
            }

            int aura = db::get_aura(g_id, u_id);
            if ((item.cost >= 0 && aura >= item.cost) || (item.cost < 0 && aura <= item.cost)) {
                db::rmv_aura(g_id, u_id, item.cost);
                db::inv_add(g_id, u_id, id);
                db::inv_eq(g_id, u_id, id);
                event.reply(dpp::message("purchased and equipped **" + item.name + "**. manage your inventory using `/inventory`.").set_allowed_mentions(false, false, false, false, {}, {}));
            } else {
                event.reply(dpp::message("not enough money.").set_flags(dpp::m_ephemeral));
            }

        } else if (sub == "sell") {
            int id = std::get<int64_t>(event.get_parameter("id"));
            if (!db::inv_has(g_id, u_id, id)) { 
                event.reply(dpp::message("you dont even have one").set_flags(dpp::m_ephemeral)); 
                return; 
            }

            auto item = db::shop_get(g_id, id);
            int refund = item.cost / 10;

            if (item.type == "role") {
                bot.guild_member_remove_role(g_id, u_id, item.role_id);
            }

            db::add_aura(g_id, u_id, refund);
            db::inv_rm(g_id, u_id, id);
            db::inv_uneq(g_id, u_id, id);
            event.reply(dpp::message("sold **" + item.name + "** for " + std::to_string(refund) + " aura."));

        } else if (sub == "admin") {
            if (!utils::is_admin(event)) {
                event.reply(dpp::message("no.").set_flags(dpp::m_ephemeral));
                return;
            }
            auto subcmd = cmd.options[0].options[0];
            if (subcmd.name == "add") {
                dpp::snowflake r_id = std::get<dpp::snowflake>(event.get_parameter("role"));
                int cost = std::get<int64_t>(event.get_parameter("cost"));
                std::string desc = "";

                auto param = event.get_parameter("desc");
                if (std::holds_alternative<std::string>(param)) desc = std::get<std::string>(param);

                int i_id = db::shop_add(g_id, "role", r_id, "<@&" + std::to_string(r_id) + ">", desc, cost, "{}");
                event.reply(dpp::message("added to shop. id: " + std::to_string(i_id)).set_flags(dpp::m_ephemeral));

            } else if (subcmd.name == "remove") {
                int id = std::get<int64_t>(event.get_parameter("id"));
                db::shop_rmv(g_id, id);
                event.reply(dpp::message("item reoved.").set_flags(dpp::m_ephemeral));
            } else if (subcmd.name == "listall") {
                int page = std::get<int64_t>(event.get_parameter("page"));
                page--;
                auto items_full = db::shop_get_all(g_id, false);
                size_t start_idx = static_cast<size_t>(page) * config::BAZAAR_PGSZ;
                if (start_idx > items_full.size()) {
                    start_idx = items_full.size();
                }
                size_t count = std::min(static_cast<size_t>(config::BAZAAR_PGSZ), items_full.size() - start_idx);

                auto items = std::span{items_full}.subspan(start_idx, count);
                if (items.empty()) {
                    event.reply("no");
                    return;
                }
                std::string out = "";
                for (auto& i : items) {
                    std::string display = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
                    out += std::to_string(i.item_id) + " " + display + " " + std::to_string(i.cost) + " aura" + (db::shop_state(g_id, i.item_id, "active") == 1 ? "(ACTIVE) " : "(INACTIVE) " ) + 
                        (db::shop_state(g_id, i.item_id, "obtainable") == 1 ? "(OBTAINABLE)" : "(UNOBTAINABLE)");
                    if (!i.desc.empty()) out += " " + i.desc + "*";
		    out += "\n";
                }
                event.reply(dpp::message(out).set_allowed_mentions(false, false, false, false, {}, {}));
            } else if (subcmd.name == "toggle") {
                int id = std::get<int64_t>(event.get_parameter("id"));
                int curr = db::shop_state(g_id, id, "active");
                db::shop_set_int(g_id, id, "active", (curr == 1 ? 0 : 1));
                event.reply(dpp::message("done").set_allowed_mentions(false, false, false, false, {}, {}));
            } else if (subcmd.name == "obt") {
                int id = std::get<int64_t>(event.get_parameter("id"));
                int curr = db::shop_state(g_id, id, "obtainable");
                db::shop_set_int(g_id, id, "obtainable", (curr == 1 ? 0 : 1));
                event.reply(dpp::message("done"));
            } else if (subcmd.name == "price") {
                int id = std::get<int64_t>(event.get_parameter("id"));
                int cost = std::get<int64_t>(event.get_parameter("price"));
                db::shop_set_int(g_id, id, "cost", cost);
                event.reply(dpp::message("done"));
            } else if (subcmd.name == "give") {
                auto user = event.get_parameter("user");
                int id = std::get<int64_t>(event.get_parameter("id"));

                db::inv_add(g_id, std::get<dpp::snowflake>(user), id);
		event.reply(dpp::message("done"));
            } else if (subcmd.name == "take") {
                auto user = event.get_parameter("user");
                int id = std::get<int64_t>(event.get_parameter("id"));

                db::inv_rm(g_id, std::get<dpp::snowflake>(user), id);
		event.reply(dpp::message("done"));
            }
        }
    }
}
