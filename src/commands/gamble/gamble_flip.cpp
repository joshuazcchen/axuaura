#include <random>

#include "commands.h"
#include "db.h"

namespace commands {

	void gamble_do_flip(const dpp::slashcommand_t& event, dpp::cluster& bot, dpp::snowflake user_id, int64_t bet,
						int aura, const std::string& choice) {
		using sv = std::vector<std::string>;
		sv heads = {"<a:heads4:1501132228946559006>",
					"<:row1column1:1489394459748864292><:row1column2:1489394446733807882>",
					"<:row2column1:1489394473845784737><:row2column2:1489394488647614504>"};
		sv tails = {"<a:tails4:1501132214577139733>",
					"<:row1column1:1489394309198250044><:row1column2:1489394322972348416>",
					"<:row2column1:1489394338378023105><:row2column2:1489394354627022919>"};

		sv op = (choice == "heads") ? heads : tails;
		sv opp = (choice == "heads") ? tails : heads;

		thread_local std::philox4x32 rng(std::random_device{}());
		bool win = std::bernoulli_distribution(0.5)(rng);

		if (win) {
			int rslt = (int)(bet * 1.5);
			db::add_aura(event.command.guild_id, user_id, rslt);
			event.reply(dpp::message(op[0]));
			new dpp::oneshot_timer(&bot, 2, [event, bet, aura, op](dpp::timer) {
				event.edit_original_response(
					dpp::message(op[1] + "\n" + op[2] + " \nyour win!\n" + std::to_string(aura) + "->" +
								 std::to_string(aura + (int)(bet * 0.5)) + " (+" + std::to_string(bet / 2) + ")"));
			});
		} else {
			event.reply(dpp::message(opp[0]));
			new dpp::oneshot_timer(&bot, 2, [event, bet, aura, opp](dpp::timer) {
				event.edit_original_response(
					dpp::message(opp[1] + "\n" + opp[2] + " \nyour lose!\n" + std::to_string(aura) + "->" +
								 std::to_string(aura - (int)bet) + " (-" + std::to_string(bet) + ")"));
			});
		}
	}

} // namespace commands
