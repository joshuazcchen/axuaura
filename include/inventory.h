// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include "commands.h"
#include <atomic>
#include <vector>

namespace db {
	struct InvItem;
}

namespace commands {
	constexpr int INV_PGSZ = 14;
	extern std::atomic_bool inv_rendering;

	void add_inv_buttons(dpp::message& msg, const std::vector<db::InvItem>& slice, int page, int total_pages);
	void handle_inv_view(const dpp::slashcommand_t& event, dpp::cluster& bot);
	void handle_inv_equip(const dpp::slashcommand_t& event, dpp::cluster& bot);
	void handle_inv_unequip(const dpp::slashcommand_t& event, dpp::cluster& bot);
} // namespace commands
