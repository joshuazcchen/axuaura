// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#pragma once
#include <dpp/dpp.h>

namespace voice {
	void handle(const dpp::voice_state_update_t& event, dpp::cluster& bot);
}
