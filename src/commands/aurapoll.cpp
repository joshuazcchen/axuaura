#include "commands.h"
#include "db.h"

namespace commands {
dpp::slashcommand bet_def(dpp::cluster& bot)
{
    return dpp::slashcommand("bet", "bet aura", bot.me.id)
        .set_dm_permission(false)
        .add_option(dpp::command_option(dpp::co_sub_command, "place", "place bet")
                .add_option(dpp::command_option(dpp::co_integer, "id", "wager id", true))
                .add_option(dpp::command_option(dpp::co_string, "side", "who you think will win", true))
                .add_option(dpp::command_option(dpp::co_integer, "bet", "how much?", true)))
        .add_option(dpp::command_option(dpp::co_sub_command, "start", "start bet")
                .add_option(dpp::command_option(dpp::co_string, "title", "title", true))
                .add_option(dpp::command_option(dpp::co_string, "options", "options (comma separated)", true)))
        .add_option(dpp::command_option(dpp::co_sub_command, "list", "active polls and amts")
                .add_option(dpp::command_option(dpp::co_integer, "id", "poll id", false)))
        .add_option(dpp::command_option(dpp::co_sub_command, "end", "end bet")
                .add_option(dpp::command_option(dpp::co_integer, "id", "wager id", true))
                .add_option(dpp::command_option(dpp::co_string, "winner", "winning op", true)));
}

void handle_bet(const dpp::slashcommand_t& event, dpp::cluster& bot)
{
    auto cmd = event.command.get_command_interaction();
    std::string scmd = cmd.options[0].name;
    if (scmd == "place") {
        int64_t p_id = std::get<int64_t>(event.get_parameter("id"));
        std::string side = std::get<std::string>(event.get_parameter("side"));
        int64_t amt = std::get<int64_t>(event.get_parameter("bet"));
        if (amt < 0) {
            event.reply(dpp::message("brokie.").set_flags(dpp::m_ephemeral));
            return;
        }

        std::string ops = db::p_get_poll_ops(p_id);
        if (ops.find(side) == std::string::npos) {
            event.reply(dpp::message("not valid: " + ops).set_flags(dpp::m_ephemeral));
            return;
        }

        int aura = db::get_aura(event.command.guild_id, event.command.get_issuing_user().id);
        if (aura < amt) {
            event.reply(dpp::message("too broke for this").set_flags(dpp::m_ephemeral));
        }
        db::rmv_aura(event.command.guild_id, event.command.get_issuing_user().id, amt);
        db::p_place_bet(p_id, event.command.get_issuing_user().id, side, amt);
        event.reply(dpp::message("placed bet of " + std::to_string(amt) + " on " + side).set_flags(dpp::m_ephemeral));
    } else if (scmd == "list") {
        dpp::command_value p_id_p = event.get_parameter("id");
        bool specific = std::holds_alternative<int64_t>(p_id_p);

        std::vector<db::Poll> polls;

        if (specific) {
            int64_t p_id = std::get<int64_t>(p_id_p);
            db::Poll p = db::p_get_poll((int)p_id);
            if (p.p_id != -1)
                polls.push_back(p);
        } else {
            polls = db::p_get_polls(event.command.guild_id);
        }

        if (polls.empty()) {
            event.reply(dpp::message("no bets rn").set_flags(dpp::m_ephemeral));
            return;
        }
        std::string stats = "this command does not really work rn for the amts\n ill fix it later -corgi\n";
        for (const auto& p : polls) {
            stats += "\n**" + std::to_string(p.p_id) + ":** " + p.title + "\n";
            std::stringstream ss(p.ops);
            std::string op;
            long total = 0;
            while (std::getline(ss, op, ',')) {
                std::string optrim = op;
                optrim.erase(0, optrim.find_first_not_of(" \t\n\r"));
                optrim.erase(optrim.find_last_not_of(" \t\n\r") + 1);
                if (optrim.empty())
                    continue;
                long subtotal = db::p_get_op_pot(p.p_id, op);
                stats += op + ": " + std::to_string(subtotal) + "\n";
                total += subtotal;
            }
        }
        event.reply(dpp::message(stats));
    } else if (scmd == "start") {
        if (!event.command.get_resolved_permission(event.command.usr.id).has(dpp::p_administrator)) {
            event.reply(dpp::message("no").set_flags(dpp::m_ephemeral));
            return;
        }
        std::string title = std::get<std::string>(event.get_parameter("title"));
        std::string ops = std::get<std::string>(event.get_parameter("options"));
        int p_id = db::p_set_poll(event.command.guild_id, title, ops);

        event.reply(dpp::message("poll: " + title + "\nid: " + std::to_string(p_id) + "\noptions: " + ops + "\nplace bets using /bet place"));
    } else if (scmd == "end") {
        if (!event.command.get_resolved_permission(event.command.usr.id).has(dpp::p_administrator)) {
            event.reply(dpp::message("no").set_flags(dpp::m_ephemeral));
            return;
        }
        int64_t p_id = std::get<int64_t>(event.get_parameter("id"));
        std::string win = std::get<std::string>(event.get_parameter("winner"));

        if (!db::p_get_poll_end(p_id)) {
            event.reply(dpp::message("poll dne").set_flags(dpp::m_ephemeral));
            return;
        }
        long pot = db::p_get_all_pot(p_id);
        long win_pot = db::p_get_op_pot(p_id, win);
        if (win_pot == 0) {
            db::p_end_poll(p_id);
            event.reply("no winners");
            return;
        }
        auto winners = db::p_get_op_users(p_id, win);
        int count = 0;
        for (auto& [u_id, amt] : winners) {
            count++;
            double share = (double)amt / win_pot;
            int payout = (int)(share * pot);
            db::add_aura(event.command.guild_id, dpp::snowflake(u_id), payout);
        }
        db::p_end_poll(p_id);
        event.reply(win + " won, total of " + std::to_string(pot) + " paid out to " + std::to_string(count) + " winners");
    }
}
} // namespace commands
