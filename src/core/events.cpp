#include "events.h"
#include "messages.h"
#include "voice.h"

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot) {
		messages::handle(event, bot);
	}
}
