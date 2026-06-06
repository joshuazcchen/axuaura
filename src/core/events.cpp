// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "events.h"

#include "message.h"
#include "voice.h"

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot) { message::handle(event, bot); }
	void handle_voice(const dpp::voice_state_update_t& event, dpp::cluster& bot) { voice::handle(event, bot); }
} // namespace events
