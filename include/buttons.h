#pragma once
#include <dpp/dpp.h>
#include <functional>
#include <map>

namespace buttons {
	// Button ID prefix -> handler mapping
	using ButtonHandler = std::function<void(const dpp::button_click_t&, dpp::cluster&)>;

	/**
	 * Register a button handler for a given ID prefix
	 * Example: register_handler("duel_", handle_duel_buttons)
	 * This will route all buttons starting with "duel_" to that handler
	**/
	void register_handler(const std::string& prefix, ButtonHandler handler);

	/**
	 * Create a button component with proper styling and ID
	**/
	dpp::component create_button(const std::string& id, const std::string& label, 
								const std::string& style = "primary", 
								const std::string& emoji = "");

	/**
	 * Main button click dispatcher (called from main.cpp)
	**/
	void handle_button_click(const dpp::button_click_t& event, dpp::cluster& bot);
}
