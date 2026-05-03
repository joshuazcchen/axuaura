#include "commands.h"
#include "db.h"
#include <algorithm>

namespace commands {

    dpp::slashcommand bazaar_def(dpp::cluster& bot) {
        return dpp::slashcommand("bazaar", "IT'S HERE!", bot.me.id)
            .add_option(dpp::command_option(dpp::co_sub_command, "list", "browse the wares"))
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
            );
    }

    void handle_bazaar(const dpp::slashcommand_t& event, dpp::cluster& bot) {
        auto cmd = event.command.get_command_interaction();
        std::string sub = cmd.options[0].name;
        dpp::snowflake g_id = event.command.guild_id;
        dpp::snowflake u_id = event.command.get_issuing_user().id;

        if (sub == "list") {
            auto items = db::shop_get_all(g_id);
            std::string out = "# the bazaar\n\n";
            for (auto& i : items) {
                std::string display = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
                out += "**id: " + std::to_string(i.item_id) + "**, " + display + ", **price (tax inclusive): ** " + std::to_string(i.cost) + " aura\n";
                if (!i.desc.empty()) out += "> *" + i.desc + "*\n";
            }
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
                event.reply(dpp::message("purchased **" + item.name + "**.").set_allowed_mentions(false, false, false, false, {}, {}));
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
            event.reply(dpp::message("sold **" + item.name + "** for " + std::to_string(refund) + " aura."));

        } else if (sub == "admin") {
            auto subcmd = cmd.options[0].options[0];
            if (subcmd.name == "add") {
                dpp::snowflake r_id = std::get<dpp::snowflake>(event.get_parameter("role"));
                int cost = std::get<int64_t>(event.get_parameter("cost"));
                std::string desc = "";
                
                auto param = event.get_parameter("desc");
                if (std::holds_alternative<std::string>(param)) desc = std::get<std::string>(param);
                
                int i_id = db::shop_add(g_id, "role", r_id, "<@" + std::to_string(r_id) + ">", desc, cost, "{}");
                event.reply(dpp::message("added to shop. id: " + std::to_string(i_id)).set_flags(dpp::m_ephemeral));

            } else if (subcmd.name == "remove") {
                int id = std::get<int64_t>(event.get_parameter("id"));
                db::shop_rmv(g_id, id);
                event.reply(dpp::message("item reoved.").set_flags(dpp::m_ephemeral));
            }
        }
    }
}
