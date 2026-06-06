// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "bazaar.h"
#include "commands.h"
#include "utils.h"

namespace commands {

	dpp::slashcommand bazaar_def(dpp::cluster& bot) {
		auto admin =
			dpp::command_option(dpp::co_sub_command_group, "admin", "Admin commands")
				.add_option(
					dpp::command_option(dpp::co_sub_command, "add", "add an item to the shop")
						.add_option(dpp::command_option(dpp::co_string, "type", "role / banner / xp_boost", true)
										.add_choice({"Role", std::string("role")})
										.add_choice({"Banner", std::string("banner")})
										.add_choice({"XP Boost", std::string("xp_boost")}))
						.add_option(dpp::command_option(dpp::co_integer, "cost", "cost in aura", true))
						.add_option(dpp::command_option(dpp::co_role, "role", "role (role type)", false))
						.add_option(dpp::command_option(dpp::co_string, "button_colour", "colour of button", false)
										.add_choice({"blurple", std::string("primary")})
										.add_choice({"green", std::string("success")})
										.add_choice({"grey", std::string("secondary")})
										.add_choice({"red", std::string("danger")}))
						.add_option(
							dpp::command_option(dpp::co_string, "colour1", "role colour 1 (hex e.g. #ffffff)", false))
						.add_option(
							dpp::command_option(dpp::co_string, "colour2", "role colour 2 for gradient (hex)", false))
						.add_option(
							dpp::command_option(dpp::co_string, "filename", "file in assets/bg/bazaar/ ", false))
						.add_option(
							dpp::command_option(dpp::co_string, "artist", "artist credit shown on level card", false))
						.add_option(
							dpp::command_option(dpp::co_boolean, "invert", "invert level card text colours", false))

						.add_option(dpp::command_option(dpp::co_string, "multiplier", "e.g. 2.0 (xp_boost)", false))
						.add_option(
							dpp::command_option(dpp::co_integer, "duration", "hours 0=permanent (xp_boost)", false))
						.add_option(dpp::command_option(dpp::co_string, "name", "display name", false))
						.add_option(dpp::command_option(dpp::co_string, "desc", "description", false))
						.add_option(dpp::command_option(dpp::co_boolean, "pinned", "always shown in bazaar?", false)))
				.add_option(
					dpp::command_option(dpp::co_sub_command, "remove", "remove item from shop")
						.add_option(dpp::command_option(dpp::co_integer, "id", "item id or rotation slot", true))
						.add_option(dpp::command_option(dpp::co_boolean, "relative", "id is a rotation slot?", false))
						.add_option(dpp::command_option(dpp::co_integer, "compensation",
														"aura to give each owner (0 = none)", false)))
				.add_option(dpp::command_option(dpp::co_sub_command, "listall", "list all items")
								.add_option(dpp::command_option(dpp::co_integer, "page", "page", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "toggle", "toggle active")
								.add_option(dpp::command_option(dpp::co_integer, "id", "item id", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "obt", "toggle obtainable")
								.add_option(dpp::command_option(dpp::co_integer, "id", "item id", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "sellable", "toggle sellable")
								.add_option(dpp::command_option(dpp::co_integer, "id", "item id", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "price", "set price")
								.add_option(dpp::command_option(dpp::co_integer, "id", "item id", true))
								.add_option(dpp::command_option(dpp::co_integer, "price", "price", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "give", "give item to user")
								.add_option(dpp::command_option(dpp::co_user, "user", "user", true))
								.add_option(dpp::command_option(dpp::co_integer, "id", "item id", true)))
				.add_option(
					dpp::command_option(dpp::co_sub_command, "take", "take item from user")
						.add_option(dpp::command_option(dpp::co_user, "user", "user", true))
						.add_option(dpp::command_option(dpp::co_integer, "id", "inventory pos or item id", true))
						.add_option(
							dpp::command_option(dpp::co_boolean, "relative", "id is inventory position?", false)))
				.add_option(dpp::command_option(dpp::co_sub_command, "viewinv", "view user inventory")
								.add_option(dpp::command_option(dpp::co_user, "user", "user", true))
								.add_option(dpp::command_option(dpp::co_integer, "page", "page", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "reroll", "reroll a rotation slot")
								.add_option(dpp::command_option(dpp::co_integer, "slot", "slot number", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "setchannel", "set bazaar UI channel")
								.add_option(dpp::command_option(dpp::co_channel, "channel", "channel", true)))
				.add_option(dpp::command_option(dpp::co_sub_command, "setrotation", "configure rotation counts")
								.add_option(dpp::command_option(dpp::co_integer, "positive",
																"# of rotating positive items (left)", false))
								.add_option(dpp::command_option(dpp::co_integer, "negative",
																"# of rotating negative items (right)", false))
								.add_option(dpp::command_option(dpp::co_integer, "refresh_hours",
																"hours between rotations", false)))
				.add_option(dpp::command_option(dpp::co_sub_command, "refresh", "force refresh bazaar"));

		return dpp::slashcommand("bazaar", "The Bazaar!", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "sell", "sell an item from your inventory")
							.add_option(dpp::command_option(dpp::co_integer, "id", "inventory position", true)))
			.add_option(admin);
	}

	void handle_bazaar(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sub = event.command.get_command_interaction().options[0].name;
		if (sub == "sell")
			handle_bazaar_sell(event, bot);
		else if (sub == "admin")
			handle_bazaar_admin(event, bot);
	}

} // namespace commands
