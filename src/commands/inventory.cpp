#include "commands.h"
#include "db.h"

namespace commands {

    dpp::slashcommand inventory_def(dpp::cluster& bot) {
        return dpp::slashcommand("inventory", "view/manage your items", bot.me.id)
            .add_option(dpp::command_option(dpp::co_sub_command, "view", "view your items"))
            .add_option(dpp::command_option(dpp::co_sub_command, "unequip", "unequip an item")
                .add_option(dpp::command_option(dpp::co_integer, "id", "item to unequip", true)))
            .add_option(dpp::command_option(dpp::co_sub_command, "equip", "equip item")
                .add_option(dpp::command_option(dpp::co_integer, "id", "item to equip", true)));
    }

    void handle_inventory(const dpp::slashcommand_t& event, dpp::cluster& bot) {
        auto cmd = event.command.get_command_interaction();
        std::string sub = cmd.options[0].name;
        dpp::snowflake g_id = event.command.guild_id;
        dpp::snowflake u_id = event.command.get_issuing_user().id;

        if (sub == "view") {
            auto items = db::inv_get_user(g_id, u_id);
            
            std::string out = "# your inventory:\n\n";
            if (items.empty()) {
                out += "*nothing here*";
            } else {
                for (auto& i : items) {
                    std::string display = (i.type == "role") ? "<@&" + std::to_string(i.role_id) + ">" : i.name;
                    out += "**" + std::to_string(i.item_id) + "**. " + display + (i.equipped ? " `(equipped)`\n" : "\n");
                }
            }
            
            event.reply(dpp::message(out).set_allowed_mentions(false, false, false, false, {}, {}));
            
        } else if (sub == "equip") {
            int id = std::get<int64_t>(event.get_parameter("id"));
            if (!db::inv_has(g_id, u_id, id)) { 
                event.reply(dpp::message("loser you dont own this").set_flags(dpp::m_ephemeral)); 
                return; 
            }
            
            auto item = db::shop_get(g_id, id);
            if (item.type == "role") {
                bot.guild_member_add_role(g_id, u_id, item.role_id);
            }
            
            db::inv_eq(g_id, u_id, id);
            event.reply(dpp::message("equipped **" + item.name + "**.").set_allowed_mentions(false, false, false, false, {}, {}));

        } else if (sub == "unequip") {
            int id = std::get<int64_t>(event.get_parameter("id"));
            if (!db::inv_has(g_id, u_id, id)) { 
                event.reply(dpp::message("you dont even own this").set_flags(dpp::m_ephemeral)); 
                return; 
            }

            auto item = db::shop_get(g_id, id);
            if (item.type == "role") {
                bot.guild_member_remove_role(g_id, u_id, item.role_id);
            }
            
            db::inv_uneq(g_id, u_id, id);
            event.reply(dpp::message("unequipped **" + item.name + "**.").set_allowed_mentions(false, false, false, false, {}, {}));
        }
    }
}
