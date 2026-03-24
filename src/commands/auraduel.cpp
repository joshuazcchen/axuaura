#include "commands.h"
#include "db.h"
#include <variant>
#include <random>

namespace commands {

	// Duel result struct
	struct DuelResult {
		bool challenger_wins;
		int prize_amount;
		int loss_amount;
		std::string winner_mention;
		std::string loser_mention;
		std::string shame_text;
	};

	// Game logic
	DuelResult process_duel_outcome(dpp::snowflake challenger_id, dpp::snowflake opponent_id, int wager) {
		int challenger_aura = db::get_aura(challenger_id);
		int opponent_aura = db::get_aura(opponent_id);
		
		// Use absolute values to calculate win chance - further from 0 = more advantage
		int total_aura = std::abs(challenger_aura) + std::abs(opponent_aura);
		int challenger_win_chance = (total_aura > 0) ? (std::abs(challenger_aura) * 100) / total_aura : 50;

		// better random generator mr Corgi, inspired from ur events.cpp
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, 99);
		int roll = distribution(gen);
		bool challenger_wins = roll < challenger_win_chance;

		if (challenger_wins) {
			db::add_aura(challenger_id, wager);
			db::rmv_aura(opponent_id, wager);
		} else {
			int challenger_loss = (wager * 3) / 2;  // 1.5x penalty for losing as challenger
			db::add_aura(opponent_id, wager);
			db::rmv_aura(challenger_id, challenger_loss);
		}

		int challenger_loss = challenger_wins ? 0 : (wager * 3) / 2;
		int loss_amount = challenger_wins ? wager : challenger_loss;

		std::string shame_text;
		if (challenger_wins) {
			shame_text = "Got absolutely OBLITERATED. Maybe next time twin <:skem:1485404723501863085>.";
		} else {
			shame_text = "YIKES. You got clapped as the CHALLENGER. That's embarrassing <:heartem:1485404606480515172>.";
		}

		return DuelResult{
			challenger_wins,
			wager,
			loss_amount,
			challenger_wins ? ("<@" + challenger_id.str() + ">") : ("<@" + opponent_id.str() + ">"),
			challenger_wins ? ("<@" + opponent_id.str() + ">") : ("<@" + challenger_id.str() + ">"),
			shame_text
		};
	}

	dpp::embed get_duel_result_embed(const DuelResult& result) {
		return dpp::embed()
			.set_color(result.challenger_wins ? dpp::colors::green : dpp::colors::red)
			.set_title("<:duelem:1485404560292974713> DUEL RESULTS <:duelem:1485404560292974713>")
			.set_description(result.winner_mention + " **WINS** against " + result.loser_mention + "!")
			.add_field("Prize", "+" + std::to_string(result.prize_amount) + " AURA", true)
			.add_field("Loss", "-" + std::to_string(result.loss_amount) + " AURA" + (!result.challenger_wins ? " (challenger penalty!)" : ""), true)
			.add_field("<:heartem:1485404606480515172> Shame", result.shame_text, false);
	}

	// Button handlers
	void handle_duel_accept(const dpp::button_click_t& event, dpp::cluster& bot, dpp::snowflake challenger_id, dpp::snowflake opponent_id, int wager) {
		dpp::snowflake user_id = event.command.get_issuing_user().id;

		// correct opponent check
		if (user_id != opponent_id) {
			event.reply(dpp::message("Only <@" + opponent_id.str() + "> can accept this duel!").set_flags(dpp::m_ephemeral));
			return;
		}

		event.reply(dpp::message("Duel accepted! Results incoming").set_flags(dpp::m_ephemeral));

		// Process duel
		auto result = process_duel_outcome(challenger_id, opponent_id, wager);
		dpp::embed result_embed = get_duel_result_embed(result);
		bot.message_create(dpp::message(event.command.channel_id, result_embed));
	}

	void handle_duel_decline(const dpp::button_click_t& event, dpp::cluster& bot, dpp::snowflake opponent_id) {
		dpp::snowflake user_id = event.command.get_issuing_user().id;

		// Only the opponent can decline
		if (user_id != opponent_id) {
			event.reply(dpp::message("Only <@" + opponent_id.str() + "> can decline this duel!").set_flags(dpp::m_ephemeral));
			return;
		}

		event.reply(dpp::message(":octagonal_sign: Duel declined!").set_flags(dpp::m_ephemeral));
	}

	void handle_duel_buttons(const dpp::button_click_t& event, dpp::cluster& bot) {
		std::string button_id = event.custom_id;

		// Handle duel accept buttons
		if (button_id.find("duel_accept_") == 0) {
			std::string ids = button_id.substr(12); // Remove "duel_accept_"
			size_t first_underscore = ids.find('_');
			size_t second_underscore = ids.find('_', first_underscore + 1);
			
			if (first_underscore != std::string::npos && second_underscore != std::string::npos) {
				dpp::snowflake challenger_id = std::stoull(ids.substr(0, first_underscore));
				dpp::snowflake opponent_id = std::stoull(ids.substr(first_underscore + 1, second_underscore - first_underscore - 1));
				int wager = std::stoi(ids.substr(second_underscore + 1));
				handle_duel_accept(event, bot, challenger_id, opponent_id, wager);
			}
			return;
		}

		// Handle duel decline buttons
		if (button_id.find("duel_decline_") == 0) {
			std::string ids = button_id.substr(13); // Remove "duel_decline_"
			size_t underscore_pos = ids.find('_');
			if (underscore_pos != std::string::npos) {
				dpp::snowflake opponent_id = std::stoull(ids.substr(underscore_pos + 1));
				handle_duel_decline(event, bot, opponent_id);
			}
			return;
		}
	}

	dpp::slashcommand auraduel_def(dpp::cluster& bot) {
		int min_wager = db::get_setting_int("duelminwager", 500);
		return dpp::slashcommand("duel", "Challenge someone to a duel for aura", bot.me.id)
			.set_dm_permission(false)
			.add_option(dpp::command_option(dpp::co_user, "opponent", "who do you want to duel?", true))
			.add_option(dpp::command_option(dpp::co_integer, "wager", "how much aura to wager (min " + std::to_string(min_wager) + ")", false));
	}

	void handle_auraduel(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake challenger_id = event.command.get_issuing_user().id;
		dpp::snowflake opponent_id = std::get<dpp::snowflake>(event.get_parameter("opponent"));
		int64_t wager = 500; // default wager
		int min_wager = db::get_setting_int("duelminwager", 500);

		dpp::command_value wager_param = event.get_parameter("wager");
		if (!std::holds_alternative<std::monostate>(wager_param)) {
			wager = std::get<int64_t>(wager_param);
		}

		// Validation
		if (opponent_id == challenger_id) {
			event.reply(dpp::message("You can't duel yourself, lol").set_flags(dpp::m_ephemeral));
			return;
		}

		if (wager < min_wager) {
			event.reply(dpp::message("Minimum wager is " + std::to_string(min_wager) + " aura").set_flags(dpp::m_ephemeral));
			return;
		}

		int challenger_aura = db::get_aura(challenger_id);
		int opponent_aura = db::get_aura(opponent_id);

		// same sign duel only
		bool challenger_negative = challenger_aura < 0;
		bool opponent_negative = opponent_aura < 0;
		if (challenger_negative != opponent_negative) {
			event.reply(dpp::message("Tricky guy, u can only duel those with same aura sign!").set_flags(dpp::m_ephemeral));
			return;
		}

		// Check wager using absolute values
		if (std::abs(challenger_aura) < wager) {
			event.reply(dpp::message("You don't have enough aura to wager that much!").set_flags(dpp::m_ephemeral));
			return;
		}

		dpp::embed invite_embed = dpp::embed()
			.set_color(dpp::colors::red)
			.set_title("<:duelem:1485404560292974713> DUEL CHALLENGE <:duelem:1485404560292974713>")
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
					.set_id("duel_accept_" + challenger_id.str() + "_" + opponent_id.str() + "_" + std::to_string(wager))
					.set_label(":white_check_mark: Accept")
					.set_style(dpp::cos_success)
					)
				.add_component(
					dpp::component()
					.set_type(dpp::cot_button)
					.set_id("duel_decline_" + challenger_id.str() + "_" + opponent_id.str())
					.set_label(":octagonal_sign: Decline")
					.set_style(dpp::cos_danger)
					)
				);

		event.reply(duel_msg);
	}
}
