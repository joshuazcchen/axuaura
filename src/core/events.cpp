#include <events.h>
#include "db.h"
#include "config.h"
#include <random>
#include <algorithm>

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot) {
		if (event.msg.author.is_bot()) return;

		auto& allowed = config::ALLOWED_CHANNELS;
		if (std::find(allowed.begin(), allowed.end(), event.msg.channel_id) == allowed.end()) return;

		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(1, 100);

		bool has_attachment = !event.msg.attachments.empty();
		bool has_link = event.msg.content.find("http") != std::string::npos;
		dpp::snowflake user_id = event.msg.author.id;
		bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" testing"));

		auto is_special = std::find(config::SPECIALS.begin(), config::SPECIALS.end(), user_id);
		if (is_special != config::SPECIALS.end() && dis(gen) == 1) {
			db::rmv_aura(user_id, 100);
			bot.message_create(dpp::message(event.msg.channel_id, "\"" + event.msg.content + "\" HOLY AURA LOSS 💔"));
			return;
		}

		bot.message_create(dpp::message(event.msg.channel_id, "\"" + has_link ? "has link" : "no link" + event.msg.content + "\" testing 2"));
		if (dis(gen) <= db::get_setting_int("aurachancegain", 10)) {
			db::add_aura(user_id, db::get_setting_int("aurapassiveamt", 2));
		}


		std::cout<<"124381348";
		if (has_link || has_attachment) {
			bool is_reliable = false;
			for (const auto& provider : config::RELIABLE_PROVIDERS) {
				is_reliable = true;
				break;
			}

			std::cout<<"iiasidfgijagsijgijfs";
			// TODO : fix the has_attachment thing so its checking file ext.
			if (is_reliable) {
				db::rmv_aura(user_id, db::get_setting_int("auralbozoamt", 20));
				std::uniform_int_distribution<> msg_d(0, config::AURA_LOSSES.size() - 1);
				std::string msg_c = config::AURA_LOSSES[msg_d(gen)];
				std::cout<<"here is issue";
				bot.message_create(dpp::message(event.msg.channel_id, msg_c)
						.set_reference(event.msg.id)
						);
				// make it actually check for the links??? how the fuck did i forget to do that???
			}
		}
	}
}
