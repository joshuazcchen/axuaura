#include "buttons.h"
#include <map>

namespace buttons {
	// Registry of button handlers by prefix
//	static std::map<std::string, ButtonHandler> handler_registry;
//
//	void register_handler(const std::string& prefix, ButtonHandler handler) {
//		handler_registry[prefix] = handler;
//	}
//
//	dpp::component create_button(const std::string& id, const std::string& label, 
//								const std::string& style, const std::string& emoji) {
//		dpp::component btn;
//		btn.set_type(dpp::cot_button);
//		btn.set_id(id);
//		btn.set_label(label);
//
//		if (!emoji.empty()) {
//			btn.set_emoji(emoji);
//		}
//
//		// cool styles
//		if (style == "primary") btn.set_style(dpp::cos_primary);
//		else if (style == "success") btn.set_style(dpp::cos_success);
//		else if (style == "danger") btn.set_style(dpp::cos_danger);
//		else if (style == "secondary") btn.set_style(dpp::cos_secondary);
//
//		return btn;
//	}
//
//	void handle_button_click(const dpp::button_click_t& event, dpp::cluster& bot) {
//		std::string button_id = event.custom_id;
//
//		// check prefix for handler
//		for (const auto& [prefix, handler] : handler_registry) {
//			if (button_id.find(prefix) == 0) {
//				handler(event, bot);
//				return;
//			}
//		}
//
//		// No handler found - u could log this mr Corgi
//        std::cout << "No handler for button ID: " << button_id << std::endl;
//	}
}
