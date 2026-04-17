#pragma once
#include <dpp/dpp.h>

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot);
	void handle_voice(const dpp::voice_state_update_t& event, dpp::cluster& bot);
	int xp_req(int lvl);
}
