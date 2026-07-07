// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include <dpp/dpp.h>

namespace voice {
	void handle(const dpp::voice_state_update_t& event, dpp::cluster& bot);
}
