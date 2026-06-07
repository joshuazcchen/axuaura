#include "inventory.h"

namespace commands {

	dpp::slashcommand inventory_def(dpp::cluster& bot) {
		return dpp::slashcommand("inventory", "view/manage your items", bot.me.id)
			.add_option(dpp::command_option(dpp::co_sub_command, "view", "view your inventory")
					.add_option(dpp::command_option(dpp::co_integer, "page", "page", false)))
			.add_option(dpp::command_option(dpp::co_sub_command, "equip", "equip an item")
					.add_option(dpp::command_option(dpp::co_integer, "id", "inventory position (1, 2, 3…)", true)))
			.add_option(dpp::command_option(dpp::co_sub_command, "unequip", "unequip an item")
					.add_option(dpp::command_option(dpp::co_integer, "id", "inventory position (1, 2, 3…)", true)));
	}

	void handle_inventory(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		auto sub = event.command.get_command_interaction().options[0].name;
		if (sub == "view") handle_inv_view(event, bot);
		else if (sub == "equip") handle_inv_equip(event, bot);
		else if (sub == "unequip") handle_inv_unequip(event, bot);
	}

} // namespace commands
