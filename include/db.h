#pragma once
#include <dpp/dpp.h>
#include <string>

namespace db {
	void init();
	int get_aura(dpp::snowflake user_id);

	void set_aura(dpp::snowflake user_id, int amount);
	void add_aura(dpp::snowflake user_id, int amount);
	void rmv_aura(dpp::snowflake user_id, int amount);
	std::vector<std::pair<std::string, int>> get_ab(bool bottom);
	int get_total_aura();

	void set_setting(const std::string& key, const std::string& val);
	void set_setting(const std::string& key, int val);
	void set_setting(const std::string& key, bool val);
	std::string get_setting_str(const std::string&key, std::string default_val = "kai");
	int get_setting_int(const std::string& key, int default_val = 7);
	bool get_setting_bool(const std::string&key, bool default_val = false);
}
