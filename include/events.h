#pragma once
#include <dpp/dpp.h>

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot);
}
