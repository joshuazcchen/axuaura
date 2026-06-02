#include <ctime>
#include <dppp/dppp.h>

#include "bazaar.h"
#include "commands.h"
#include "db.h"
#include "image.h"
#include "utils.h"

namespace commands {

	void handle_bazaar_sell(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		int pos = std::get<int64_t>(event.get_parameter("id"));

		auto items = db::inv_get_user(g_id, u_id);
		if (pos < 1 || pos > (int)items.size()) {
			event.reply(dpp::message("invalid position.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto& inv = items[pos - 1];
		if (db::shop_state(g_id, inv.item_id, "sellability") == 0) {
			event.reply(dpp::message("can't sell that one.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto item = db::shop_get(g_id, inv.item_id);
		int refund = std::max(0, item.cost / 10);
		if (item.type == "role") bot.guild_member_remove_role(g_id, u_id, item.role_id);
		if (item.type == "banner" && inv.equipped) bazaar::b_banner_unequip(g_id, u_id);

		db::add_aura(g_id, u_id, refund);
		db::inv_rm_by_inv_id(inv.inv_id);
		event.reply(dpp::message("sold **" + item.name + "** for " + std::to_string(refund) + " aura."));
	}

	void handle_bazaar_button(const dpp::button_click_t& event, dpp::cluster& bot) {
		dpp::snowflake g_id = event.command.guild_id;
		dpp::snowflake u_id = event.command.get_issuing_user().id;
		std::string payload = event.custom_id.substr(8);
		size_t next = payload.find('_');
		int item_id = std::stoi(payload.substr(0, next));

		auto slots = db::bazaar_rotation_get(g_id);
		bool in_rotation = false;
		for (auto& s : slots)
			if (s.item_id == item_id) {
				in_rotation = true;
				break;
			}
		if (!in_rotation) {
			event.reply(dpp::message("no longer available.").set_flags(dpp::m_ephemeral));
			return;
		}
		auto item = db::shop_get(g_id, item_id);
		if (item.item_id == -1 || !item.active) {
			event.reply(dpp::message("item unavailable.").set_flags(dpp::m_ephemeral));
			return;
		}
		bool consumable = (item.type == "xp_boost");
		if (!consumable && db::inv_has(g_id, u_id, item_id)) {
			event.reply(dpp::message("you already own this.").set_flags(dpp::m_ephemeral));
			return;
		}
		int aura = db::get_aura(g_id, u_id);
		bool can_buy = (item.cost >= 0) ? (aura >= item.cost) : (aura <= item.cost);
		if (!can_buy) {
			event.reply(dpp::message("not enough aura (" + std::to_string(aura) + ").").set_flags(dpp::m_ephemeral));
			return;
		}
		db::rmv_aura(g_id, u_id, item.cost);

		if (item.type == "xp_boost") {
			int hours = utils::json_int(item.data, "hours", 24);
			long expires = hours > 0 ? (std::time(nullptr) + (long)hours * 3600) : 0;
			db::inv_add_timed(g_id, u_id, item_id, expires);
		} else if (item.type == "banner") {
			db::inv_uneq_all_type(g_id, u_id, "banner");
			db::inv_add(g_id, u_id, item_id);
			db::inv_eq(g_id, u_id, item_id);
			bazaar::b_banner_equip(g_id, u_id, item);
		} else {
			db::inv_add(g_id, u_id, item_id);
			db::inv_eq(g_id, u_id, item_id);
			if (item.type == "role") bot.guild_member_add_role(g_id, u_id, item.role_id);
		}
		event.reply(dpp::message("purchased **" + item.name + "**!").set_flags(dpp::m_ephemeral));
	}

} // namespace commands

namespace bazaar {

	void b_banner_equip(dpp::snowflake g_id, dpp::snowflake u_id, const db::ShopItem& item) {
		std::string u = std::to_string(u_id);
		std::string fn = utils::json_str(item.data, "file");
		std::string artist = utils::json_str(item.data, "artist");
		bool invert_val = utils::json_bool(item.data, "invert", false);
		std::string invert = invert_val ? "true" : "false";

		// why did i put this in settings instead of making it its own thing???
		db::set_setting(g_id, "bg_override_" + u, fn);
		db::set_setting(g_id, "bg_artist_" + u, artist);
		db::set_setting(g_id, "bg_invert_" + u, invert);
	}

	void b_banner_unequip(dpp::snowflake g_id, dpp::snowflake u_id) {
		std::string u = std::to_string(u_id);
		db::set_setting(g_id, "bg_override_" + u, "");
		db::set_setting(g_id, "bg_artist_" + u, "");
		db::set_setting(g_id, "bg_invert_" + u, std::string("false"));
	}

	static dpp::component_style parse_btn_style(const std::string& data, dpp::component_style def) {
		std::string s = utils::json_str(data, "button_style");
		if (s == "success") return dpp::cos_success;
		if (s == "danger") return dpp::cos_danger;
		if (s == "secondary") return dpp::cos_secondary;
		if (s == "primary") return dpp::cos_primary;
		return def;
	}

	void b_post_ui(dpp::cluster& bot, dpp::snowflake g_id) {
		std::string ch_str = db::get_setting_str(g_id, "bazaar_channel", std::string("0"));
		dpp::snowflake ch_id = std::stoull(ch_str);
		if (ch_id == 0) return;

		auto slots = db::bazaar_rotation_get(g_id);
		std::vector<db::ShopItem> pos, neg;
		long next_restock = 0;
		long refresh_s = (long)db::get_setting_int(g_id, "bazaar_refresh_hours", 168) * 3600;
		long refresh_l = (long)db::get_setting_int(g_id, "bazaar_last_refresh", 0);
		long refresh_n = refresh_l > 0 ? refresh_l + refresh_s : 0;

		for (auto& s : slots) {
			auto item = db::shop_get(g_id, s.item_id);
			if (item.item_id == -1 || !item.active) continue;
			(item.cost >= 0 ? pos : neg).push_back(item);
		}

		if (pos.empty() && neg.empty()) return;

		dppp::get_enhanced_roles(
			bot, g_id, [=, &bot](const dppp::result<std::vector<dppp::enhanced_role>>& res) mutable {
				if (res.success) {
					std::unordered_map<dpp::snowflake, dppp::enhanced_role> role_map;
					for (const auto& r : res.value) {
						role_map[r.id] = r;
					}

					auto inject_colors = [&](std::vector<db::ShopItem>& items) {
						for (auto& item : items) {
							if (item.type == "role" && role_map.contains(item.role_id)) {
								const auto& r = role_map[item.role_id];

								std::string c1 = r.colours.primary_hex_colour();
								std::string c2 = r.colours.secondary_hex_colour().value_or("");
								std::string c3 = r.colours.tertiary_colour.has_value()
													 ? dppp::to_hex_colour(r.colours.tertiary_colour.value())
													 : "";

								std::string new_keys = "";
								if (!c1.empty()) new_keys += "\"colour1\":\"" + c1 + "\",";
								if (!c2.empty()) new_keys += "\"colour2\":\"" + c2 + "\",";
								if (!c3.empty()) new_keys += "\"colour3\":\"" + c3 + "\",";

								if (!new_keys.empty()) {
									new_keys.pop_back();
									if (item.data.empty() || item.data == "{}") {
										item.data = "{" + new_keys + "}";
									} else {
										size_t brace_pos = item.data.find_last_of('}');
										if (brace_pos != std::string::npos) {
											item.data.insert(brace_pos, "," + new_keys);
										}
									}
								}
							}
						}
					};

					inject_colors(pos);
					inject_colors(neg);
				}

				auto pages = image::img_gen_bazaar(pos, neg, next_restock);
				if (pages.empty()) return;

				dpp::component pos_row, neg_row;
				pos_row.set_type(dpp::cot_action_row);
				neg_row.set_type(dpp::cot_action_row);

				auto get_safe_name = [](const auto& item, size_t max_len = 18) -> std::string {
					if (item.type == "role" && !item.name.empty() && item.name[0] == '<') {
						std::string id_str;
						for (char c : item.name) {
							if (std::isdigit(c)) { id_str += c; }
						}

						if (!id_str.empty()) {
							try {
								dpp::snowflake role_id = std::stoull(id_str);
								dpp::role* cached_role = dpp::find_role(role_id);
								if (cached_role) {
									std::string c_name = cached_role->name;
									std::erase_if(c_name,
												  [](unsigned char c) { return !(std::isalnum(c) || c == ' '); });
									return c_name.size() <= max_len ? c_name : c_name.substr(0, max_len - 3) + "...";
								}
							} catch (...) {}
						}
						return "Role";
					}
					return item.name.size() <= max_len ? item.name : item.name.substr(0, max_len - 3) + "...";
				};

				auto build_rows = [&](std::vector<db::ShopItem>& items, bool is_pos) {
					std::vector<dpp::component> rows;
					if (items.empty()) return rows;

					dpp::component current_row;
					current_row.set_type(dpp::cot_action_row);

					for (size_t i = 0; i < items.size(); ++i) {
						if (i > 0 && i % 5 == 0) {
							rows.push_back(current_row);
							current_row = dpp::component().set_type(dpp::cot_action_row);
						}

						dpp::component_style def_style = is_pos ? dpp::cos_success : dpp::cos_danger;
						auto style = parse_btn_style(items[i].data, def_style);

						std::string lbl =
							get_safe_name(items[i]) + " (" + std::to_string(std::abs(items[i].cost)) + "a)";

						dpp::component btn;
						btn.set_type(dpp::cot_button)
							.set_style(style)
							.set_label(lbl)
							.set_id("bzr_buy_" + std::to_string(items[i].item_id) + "_" + std::to_string(i) + (is_pos ? "p" : "n"));
						std::cout<<"bzr_buy_" + std::to_string(items[i].item_id)<<std::endl;

						current_row.add_component(btn);
					}
					rows.push_back(current_row);
					return rows;
				};

				auto pos_rows = build_rows(pos, true);
				auto neg_rows = build_rows(neg, false);

				auto msg = std::make_shared<dpp::message>(ch_id, "eee");
				msg->set_allowed_mentions(false, false, false, false, {}, {});

				for (size_t i = 0; i < pages.size(); ++i) {
					msg->add_file("bazaar_p" + std::to_string(i + 1) + ".png", pages[i]);
				}

				for (auto& row : pos_rows)
					msg->add_component(row);
				for (auto& row : neg_rows)
					msg->add_component(row);

				std::string old_id_str = db::get_setting_str(g_id, "bazaar_msg_id", std::string("0"));
				dpp::snowflake old_id = std::stoull(old_id_str);

				if (old_id != 0) { 
					try {
						bot.message_delete(old_id, ch_id); 
					} catch (...) {}
				}

				bot.message_create(*msg, [g_id, msg, &bot](const dpp::confirmation_callback_t& cb) {
					if (!cb.is_error()) {
						db::set_setting(g_id, "bazaar_msg_id", std::to_string(std::get<dpp::message>(cb.value).id));
					} else {
						std::cerr << cb.get_error().message << std::endl;
					}
				});
			});
	}

} // namespace bazaar
