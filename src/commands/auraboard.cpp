#include "commands.h"
#include "db.h"
#include <algorithm>

namespace commands {

    dpp::slashcommand get_auraboard_definition(dpp::cluster& bot) {
        return dpp::slashcommand("auraboard", "who has lost the most aura?", bot.me.id)
            .add_option(
                dpp::command_option(dpp::co_string, "sort", "top or bottom 10", true)
                .add_choice(dpp::command_option_choice("top", std::string("top")))
                .add_choice(dpp::command_option_choice("bottom", std::string("bottom")))
                .add_choice(dpp::command_option_choice("me", std::string("me")))
            );
    }

    void handle_auraboard(const dpp::slashcommand_t& event, dpp::cluster& bot) {
        std::string sort_mode = std::get<std::string>(event.get_parameter("sort"));
        dpp::snowflake user_id = event.command.get_issuing_user().id;

        if (sort_mode == "me") {
            int aura = db::get_aura(user_id);
            event.reply(dpp::message("You have: " + std::to_string(aura) + " aura").set_flags(dpp::m_ephemeral));
            return;
        }

        auto ab_list = db::get_ab(sort_mode == "bottom"); 

        dpp::embed embed = dpp::embed()
            .set_color(dpp::colors::white)
            .set_title(sort_mode == "bottom" ? "AURALESS" : "AURAFUL");

        embed.add_field("total aura:", std::to_string(db::get_total_aura()), true);

        std::string txt = "";
        for (const auto& row : ab_list) {
            std::string aura_val = std::to_string(row.second) + "AURA";
            txt += "<@" + row.first + ">: " + aura_val + "\n";
        }

        embed.set_description(txt);
        event.reply(dpp::message(event.command.channel_id, embed));
    }
}
