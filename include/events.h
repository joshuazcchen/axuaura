// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#pragma once
#include <dpp/dpp.h>

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot);
	void handle_voice(const dpp::voice_state_update_t& event, dpp::cluster& bot);
} // namespace events
