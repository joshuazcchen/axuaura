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
				// TODO: find proper arrows
				b.set_type(dpp::cot_button)
					.set_id("inv_prev_" + std::to_string(page))
					.set_label("<")
					.set_style(dpp::cos_secondary);
				nav_row.add_component(b);
			}
			if (page < total_pages) {
				dpp::component b;
				b.set_type(dpp::cot_button)
					.set_id("inv_next_" + std::to_string(page))
					.set_label(">")
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
		std::vector<db::InvItem> slice(all.begin() + start, all.begin() + std::min(start + INV_PGSZ, (int)all.size()));

		while (inv_rendering.exchange(true)) {}
		std::string img;
		try {
			img = image::img_gen_inventory(slice, page, total_pages, utils::get_display_name(g_id, u_id),
										   utils::get_avatar_url(u_id, 64));
		} catch (...) {}
		inv_rendering = false;
		malloc_trim(0);

		if (img.empty()) {
			event.edit_original_response(dpp::message("couldn't render inventory.").set_flags(dpp::m_ephemeral));
			return;
		}

		dpp::message msg(event.command.channel_id, "");
		msg.set_flags(dpp::m_ephemeral);
		msg.add_file("inventory.png", img);
		add_inv_buttons(msg, slice, page, total_pages);
		event.edit_original_response(msg);
	}

} // namespace commands
