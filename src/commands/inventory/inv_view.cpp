// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <dppp/dppp.h>
#include <unordered_map>

#include "inventory.h"

namespace commands {

	std::atomic_bool inv_rendering{false};

	void add_inv_buttons(dpp::message& msg, const std::vector<db::InvItem>& slice, int page, int total_pages) {
		auto build_row = [&](int row_start) {
			dpp::component row;
			row.set_type(dpp::cot_action_row);
			int row_end = std::min(row_start + 5, (int)slice.size());
			for (int i = row_start; i < row_end; ++i) {
				const auto& it = slice[i];
				dpp::component btn;
				dpp::component_style style = dpp::cos_secondary;
				if (it.equipped) {
					style = dpp::cos_success;
				} else {
					std::string bs = utils::json_str(it.data, "button_style");
					if (bs == "primary")
						style = dpp::cos_primary;
					else if (bs == "success")
						style = dpp::cos_success;
					else if (bs == "danger")
						style = dpp::cos_danger;
				}
				btn.set_type(dpp::cot_button)
					.set_id("inv_tog_" + std::to_string(page) + "_" + std::to_string(i + 1))
					.set_label(std::to_string(i + 1) + ". " + utils::get_safe_role(it.name, it.type, 10))
					.set_style(style);
				row.add_component(btn);
			}
			return row;
		};

		if (!slice.empty()) {
			for (int i = 0; i < (int)slice.size(); i += 5) {
				msg.add_component(build_row(i));
			}
		}

		if (total_pages > 1) {
			dpp::component nav_row;
			nav_row.set_type(dpp::cot_action_row);
			if (page > 1) {
				dpp::component b;
				b.set_type(dpp::cot_button)
					.set_id("inv_prev_" + std::to_string(page))
					.set_label("◀")
					.set_style(dpp::cos_secondary);
				nav_row.add_component(b);
			}
			if (page < total_pages) {
				dpp::component b;
				b.set_type(dpp::cot_button)
					.set_id("inv_next_" + std::to_string(page))
					.set_label("▶")
					.set_style(dpp::cos_secondary);
				nav_row.add_component(b);
			}
			msg.add_component(nav_row);
		}
	}

	void handle_inv_view(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto g_id = event.command.guild_id;
		auto u_id = event.command.get_issuing_user().id;
		event.thinking(true);

		auto page_param = event.get_parameter("page");
		int page = std::holds_alternative<int64_t>(page_param) ? (int)std::get<int64_t>(page_param) : 1;
		if (page < 1) page = 1;

		auto all = db::inv_get_user(g_id, u_id);
		int total_pages = std::max(1, ((int)all.size() + INV_PGSZ - 1) / INV_PGSZ);
		if (page > total_pages) page = total_pages;

		int start = (page - 1) * INV_PGSZ;
		int end = std::min(start + INV_PGSZ, (int)all.size());

		std::vector<db::InvItem> slice(all.begin() + start, all.begin() + end);
		{ std::vector<db::InvItem>{}.swap(all); }

		std::string display_name = utils::get_display_name(g_id, u_id);
		std::string avatar_url = utils::get_avatar_url(u_id, 64);

		auto rend = [page, total_pages, display_name = std::move(display_name), avatar_url = std::move(avatar_url),
					 ch_id = event.command.channel_id, event](std::vector<db::InvItem> items) {
			while (inv_rendering.exchange(true)) {}
			std::string img;
			try {
				img = image::img_gen_inventory(items, page, total_pages, display_name, avatar_url);
			} catch (...) {}
			inv_rendering = false;
			malloc_trim(0);

			if (img.empty()) {
				event.edit_original_response(dpp::message("couldn't render inventory.").set_flags(dpp::m_ephemeral));
				return;
			}

			dpp::message msg(ch_id, "");
			msg.set_flags(dpp::m_ephemeral);
			msg.add_file("inventory.png", std::move(img));
			add_inv_buttons(msg, items, page, total_pages);
			event.edit_original_response(std::move(msg));
		};

		bool has_roles = false;
		for (const auto& it : slice) {
			if (it.type == "role") {
				has_roles = true;
				break;
			}
		}

		if (!has_roles) {
			rend(std::move(slice));
			return;
		}

		dppp::get_enhanced_roles(bot, g_id,
								 [rend = std::move(rend), slice = std::move(slice)](
									 const dppp::result<std::vector<dppp::enhanced_role>>& res) mutable {
									 if (res.success) {
										 std::unordered_map<dpp::snowflake, dppp::enhanced_role> role_map;
										 role_map.reserve(res.value.size());
										 for (const auto& r : res.value)
											 role_map[r.id] = r;

										 for (auto& item : slice) {
											 if (item.type != "role" || !item.role_id) continue;
											 auto it = role_map.find(item.role_id);
											 if (it == role_map.end()) continue;

											 const auto& r = it->second;
											 std::string c1 = r.colours.primary_hex_colour();
											 std::string c2 = r.colours.secondary_hex_colour().value_or("");
											 std::string c3 =
												 r.colours.tertiary_colour.has_value()
													 ? dppp::to_hex_colour(r.colours.tertiary_colour.value())
													 : "";
											 std::string keys;

											 if (!c1.empty()) keys += "\"colour1\":\"" + c1 + "\",";
											 if (!c2.empty()) keys += "\"colour2\":\"" + c2 + "\",";
											 if (!c3.empty()) keys += "\"colour3\":\"" + c3 + "\",";

											 if (!keys.empty()) {
												 keys.pop_back();
												 if (item.data.empty() || item.data == "{}") {
													 item.data = "{" + keys + "}";
												 } else {
													 size_t p = item.data.find_last_of('}');
													 if (p != std::string::npos) item.data.insert(p, "," + keys);
												 }
											 }
										 }
									 }
									 rend(std::move(slice));
								 });
	}

} // namespace commands
