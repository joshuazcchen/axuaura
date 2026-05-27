#include <algorithm>
#include <random>

#include "commands.h"
#include "db.h"

namespace commands {

	dpp::slashcommand duel_def(dpp::cluster& bot) {
		return dpp::slashcommand("duel", "duel", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "challenge", "threaten someone")
							.add_option(dpp::command_option(dpp::co_user, "target", "whomst", true))
							.add_option(dpp::command_option(dpp::co_integer, "bet", "ante", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "accept", "fight")
							.add_option(dpp::command_option(dpp::co_user, "challenger", "who challenged you", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "decline", "coward")
							.add_option(dpp::command_option(dpp::co_user, "challenger", "who challenged you", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "cancel", "retract your duel (what a loser)"));
	}

	void handle_duel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sgn = [](int x) { return x >= 0 ? 1 : -1; };
		auto cmd = event.command.get_command_interaction().options[0];
		dpp::snowflake user_id = event.command.get_issuing_user().id;
		if (cmd.name == "challenge") {
			dpp::snowflake target = std::get<dpp::snowflake>(event.get_parameter("target"));
			int64_t bet = std::get<int64_t>(event.get_parameter("bet"));

			if (target == user_id) {
				event.reply(dpp::message("tf? you tryna duel yourself?"));
				return;
			}
			if (target == bot.me.id) {
				event.reply(dpp::message("how you gonna duel ME and expect to win?"));
				return;
			}
			if (target == 1492320672838582322) {
				event.reply(dpp::message("dont be stupid. phil wins. you lose."));
				return;
			}
			int u_aura = db::get_aura(event.command.guild_id, user_id);
			int t_aura = db::get_aura(event.command.guild_id, target);
			int u_mag = std::abs(u_aura);
			int t_mag = std::abs(t_aura);
			int penalty = (int)(bet * 1.33333);
			// in the interest of not taking someones aura away before the duel is accepted im gonna just make it so
			// that the database stores that info.
			if (bet < 0 && u_aura < 0) {
				event.reply(dpp::message("you gotta duel based on magnitude, so always use positive values :D"));
				return;
			} else if (bet < 0) {
				event.reply(dpp::message("you cannot duel negatives."));
				return;
			}

			if (u_mag < bet) {
				event.reply(dpp::message(
					"holy broke. sit and think about what youre tryna do rq because you dont got enough aura."));
				return;
			}
			if (u_mag < penalty) {
				event.reply(dpp::message("sorry man, no can do, gotta have at least " + std::to_string(penalty) +
										 " aura to make this bet. you can duel up to " +
										 std::to_string((int)(u_aura / 1.33333))));
				return;
			}
			if (t_mag < bet) {
				event.reply(dpp::message("your ~~victim~~ is too broke. pick on someone closer to your tax bracket."));
				return;
			}
			db::d_issue(event.command.guild_id, user_id, target, bet);
			event.reply(
				dpp::message(
					"<@" + std::to_string(target) + ">, hey, <@" + std::to_string(user_id) +
					"> thinks they're better than you. wanna try to prove them wrong? they're betting **" +
					std::to_string(bet) +
					"**.\n\naccept using /duel accept\n-# note that accepting a duel cancels your outgoing duels.")
					.set_allowed_mentions(true, false, false, false, {}, {}));

			dpp::snowflake ch_id = event.command.channel_id;
			dpp::snowflake g_id = event.command.guild_id;
			new dpp::oneshot_timer(&bot, 120, [&bot, user_id, target, ch_id, g_id](dpp::timer t) {
				long issue_time = db::d_time(g_id, user_id);
				if (issue_time != -1 && (std::time(nullptr) - issue_time >= 119)) {
					db::d_delete(g_id, user_id);
					bot.message_create(dpp::message(ch_id, "<@" + std::to_string(user_id) + ">'s duel to <@" +
															   std::to_string(target) + "> expired"));
				}
			});

		} else if (cmd.name == "accept") {
			dpp::snowflake c_id = std::get<dpp::snowflake>(event.get_parameter("challenger"));
			int bet = db::d_check(event.command.guild_id, c_id, user_id);
			if (bet == -1) {
				event.reply(dpp::message("they dont wanna duel u lil bro"));
				return;
			}
			if (bet < 0) {
				event.reply(dpp::message("something went wrong lol"));
				return;
			}
			int t_aura =
				db::get_aura(event.command.guild_id, user_id); // its backwards here. target is the one accepting
			int u_aura = db::get_aura(event.command.guild_id, c_id);
			int t_mag = std::abs(t_aura);
			int u_mag = std::abs(u_aura);
			int penalty = (int)(bet * 1.33333);
			if (t_mag < bet) {
				event.reply(dpp::message("you broke asf. cant afford this duel lol"));
				db::d_delete(event.command.guild_id, c_id);
				return;
			}
			if (u_mag < penalty) {
				event.reply(
					dpp::message("your challenger is broke asf what a loser lol. you win by default but they cant even "
								 "afford to pay you back."));
				db::d_delete(event.command.guild_id, c_id);
				return;
			}
			db::d_delete(event.command.guild_id, c_id);
			double prob_u = 0.5;
			if (u_mag + t_mag > 0) { prob_u = (double)u_mag / (u_mag + t_mag); }
			prob_u = std::max(0.10, std::min(0.90, prob_u));

			thread_local std::philox4x32 rng(std::random_device{}());
			std::uniform_real_distribution<double> dist(0.0, 1.0);

			double roll = dist(rng);
			bool chwin = (roll <= prob_u);
			if (chwin) {
				db::add_aura(event.command.guild_id, c_id, sgn(u_aura) * bet);
				db::rmv_aura(event.command.guild_id, user_id, sgn(t_aura) * bet);
				int new_u = db::get_aura(event.command.guild_id, c_id);
				int new_t = db::get_aura(event.command.guild_id, user_id);

				// TODO: this is completely unreadable because its temporary formatting.
				event.reply("<@" + std::to_string(c_id) + "> mogs <@" + std::to_string(user_id) + ">.\n" + "<@" +
							std::to_string(c_id) + "> now at " + std::to_string(new_u) + "\n<@" +
							std::to_string(user_id) + "> now at " + std::to_string(new_t));
			} else {
				db::rmv_aura(event.command.guild_id, c_id, sgn(u_aura) * penalty);
				db::add_aura(event.command.guild_id, user_id, sgn(t_aura) * penalty);
				int new_u = db::get_aura(event.command.guild_id, c_id);
				int new_t = db::get_aura(event.command.guild_id, user_id);

				event.reply("<@" + std::to_string(user_id) + "> ABSOLUTELY mogs <@" + std::to_string(c_id) + ">.\n" +
							"<@" + std::to_string(c_id) + "> now at " + std::to_string(new_u) +
							"(extra loss bc they LOST despite initiating the challenge, lmfao)\n<@" +
							std::to_string(user_id) + "> now at " + std::to_string(new_t));
			}
		} else if (cmd.name == "decline") {
			dpp::snowflake c_id = std::get<dpp::snowflake>(event.get_parameter("challenger"));
			if (db::d_check(event.command.guild_id, c_id, user_id) == -1) {
				event.reply(dpp::message("they didnt even wanna duel u lil bro"));
				return;
			}
			db::d_delete(event.command.guild_id, c_id);
			event.reply(dpp::message("<@" + std::to_string(user_id) + "> is too scared of <@" + std::to_string(c_id) +
									 "> and ran away from the duel"));
		} else if (cmd.name == "cancel") {
			if (!db::d_outgoing(event.command.guild_id, user_id)) {
				event.reply(dpp::message("no log of you wanting to duel someone, idk what u tryna cancel")
								.set_flags(dpp::m_ephemeral));
				return;
			}
			db::d_delete(event.command.guild_id, user_id);
			event.reply(dpp::message("you successfully chickened out of your duel"));
		}
	}
} // namespace commands
