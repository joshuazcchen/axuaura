// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include <dpp/dpp.h>

namespace events {
	void handle_message(const dpp::message_create_t& event, dpp::cluster& bot);
	void handle_voice(const dpp::voice_state_update_t& event, dpp::cluster& bot);
} // namespace events
