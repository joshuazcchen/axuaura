#pragma once
#include <dpp/dpp.h>

#include <string>

namespace utils {
	bool is_admin(const dpp::slashcommand_t& event);

	std::string json_str(const std::string& json, const std::string& key);
	bool json_bool(const std::string& json, const std::string& key, bool default_val = false);
	int json_int(const std::string& json, const std::string& key, int default_val = 0);
	double json_doub(const std::string& json, const std::string& key, double default_val = 0.0);
} // namespace utils
