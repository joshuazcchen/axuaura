#include "commands.h"
#include "db.h"
#include <random>
#include <ctime>
#include <variant>

namespace commands {

	dpp::slashcommand get_auraduel_definition(dpp::cluster& bot) {
		return dpp::slashcommand("duel", "Challenge someone to a duel for aura", bot.me.id)
			.set_dm_permission(false)
			.add_option(dpp::command_option(dpp::co_user, "opponent", "who do you want to duel?", true))
			.add_option(dpp::command_option(dpp::co_integer, "wager", "how much aura to wager (min 10)", false));
	}

	void handle_auraduel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake challenger_id = event.command.get_issuing_user().id;
		dpp::snowflake opponent_id = std::get<dpp::snowflake>(event.get_parameter("opponent"));
		int64_t wager = 50; // default wager

		dpp::command_value wager_param = event.get_parameter("wager");
		if (!std::holds_alternative<std::monostate>(wager_param)) {
			wager = std::get<int64_t>(wager_param);
		}

		// Validation
		if (opponent_id == challenger_id) {
			event.reply(dpp::message("You can't duel yourself, lol").set_flags(dpp::m_ephemeral));
			return;
		}

		if (wager < 10) {
			event.reply(dpp::message("Minimum wager is 10 aura").set_flags(dpp::m_ephemeral));
			return;
		}

		int challenger_aura = db::get_aura(challenger_id);
		int opponent_aura = db::get_aura(opponent_id);

		if (challenger_aura < wager) {
			event.reply(dpp::message("You don't have enough aura to wager that much!").set_flags(dpp::m_ephemeral));
			return;
		}

		dpp::embed invite_embed = dpp::embed()
			.set_color(dpp::colors::red)
			.set_title("⚔️ DUEL CHALLENGE")
			.set_description("<@" + challenger_id.str() + "> challenges <@" + std::to_string(opponent_id) + "> to a duel!")
			.add_field("Wager", std::to_string(wager) + " AURA", true)
			.add_field("Your Aura", std::to_string(opponent_aura) + " AURA", true)
			.add_field("Challenger Aura", std::to_string(challenger_aura) + " AURA", true)
			.set_footer(dpp::embed_footer()
					.set_text("Accept or decline the challenge below")
					.set_icon("https://cdn-icons-png.flaticon.com/512/595/595533.png"));

		dpp::message duel_msg(event.command.channel_id, invite_embed);
		duel_msg.add_component(
				dpp::component()
				.add_component(
					dpp::component()
					.set_type(dpp::cot_button)
					.set_id("duel_accept_" + challenger_id.str() + "_" + opponent_id.str())
					.set_label("⚡ Accept")
					.set_style(dpp::cos_success)
					)
				.add_component(
					dpp::component()
					.set_type(dpp::cot_button)
					.set_id("duel_decline_" + challenger_id.str() + "_" + opponent_id.str())
					.set_label("✋ Decline")
					.set_style(dpp::cos_danger)
					)
				);

		event.reply(duel_msg);
	}

	void process_duel_result(dpp::cluster& bot, dpp::snowflake channel_id, dpp::snowflake challenger_id, dpp::snowflake opponent_id, int wager) {
		int challenger_aura = db::get_aura(challenger_id);
		std::cout<<"made it here"<<std::endl;
		int opponent_aura = db::get_aura(opponent_id);
		// TODO: fix the potential divide by zero here
		// TODO TODO: this also is the thing preventing negative aura pvp basically
		int total_aura = challenger_aura + opponent_aura;
		int challenger_win_chance = (total_aura > 0) ? (challenger_aura * 100) / total_aura : 50;

		// TODO: wtf is a hash in this context why not use rd
		size_t id_seed = std::hash<std::string>{}(challenger_id.str() + opponent_id.str());
		std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)) + static_cast<unsigned>(id_seed));
		std::uniform_int_distribution<int> distribution(0, 99);
		int roll = distribution(rng);
		bool challenger_wins = roll < challenger_win_chance;

		if (challenger_wins) {
			db::add_aura(challenger_id, wager);
			db::rmv_aura(opponent_id, wager);
		} else {
			int challenger_loss = (wager * 3) / 2;  // 1.5x penalty for losing as challenger
			db::add_aura(opponent_id, wager);
			db::rmv_aura(challenger_id, challenger_loss);
		}

		std::string winner = challenger_wins ? ("<@" + challenger_id.str() + ">") : ("<@" + opponent_id.str() + ">");
		std::string loser = challenger_wins ? ("<@" + opponent_id.str() + ">") : ("<@" + challenger_id.str() + ">");
		int challenger_loss = challenger_wins ? 0 : (wager * 3) / 2;
		int loss_amount = challenger_wins ? wager : challenger_loss;

		// Shame text for losers
		std::string shame_text;
		if (challenger_wins) {
			shame_text = "Got absolutely OBLITERATED. Better luck next time, I guess? 💀";
		} else {
			shame_text = "YIKES. You got clapped as the CHALLENGER. That's embarrassing. 😬";
		}

		dpp::embed result_embed = dpp::embed()
			.set_color(challenger_wins ? dpp::colors::green : dpp::colors::red)
			.set_title("⚔️ DUEL RESULTS")
			.set_description(winner + " **WINS** against " + loser + "!")
			.add_field("Prize", "+" + std::to_string(wager) + " AURA", true)
			.add_field("Loss", "-" + std::to_string(loss_amount) + " AURA" + (!challenger_wins ? " (challenger penalty!)" : ""), true)
			.add_field("💔 Shame", shame_text, false);

		bot.message_create(dpp::message(channel_id, result_embed));
	}
}
