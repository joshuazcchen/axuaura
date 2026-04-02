#pragma once
#include <dpp/dpp.h>
#include <string>

namespace db {
	void init();
	int get_aura(dpp::snowflake user_id);

	void set_aura(dpp::snowflake user_id, int amount);
	void add_aura(dpp::snowflake user_id, int amount);
	void rmv_aura(dpp::snowflake user_id, int amount);
	std::vector<std::pair<std::string, int>> get_ab(int limit, bool bottom);
	int get_total_aura();

	void set_setting(const std::string& key, const std::string& val);
	void set_setting(const std::string& key, int val);
	void set_setting(const std::string& key, bool val);
	std::string get_setting_str(const std::string&key, std::string default_val = "kai");
	int get_setting_int(const std::string& key, int default_val = 7);
	bool get_setting_bool(const std::string&key, bool default_val = false);

	struct Poll {
		int p_id;
		std::string title;
		std::string ops;
	};

	std::vector<Poll> p_get_polls();
	Poll p_get_poll(int p_id);
	int p_set_poll(std::string title, std::string ops);
	bool p_get_poll_end(int p_id);
	std::string p_get_poll_ops(int p_id);
	void p_place_bet(int p_id, dpp::snowflake u_id, std::string op, int amt);
	long p_get_all_pot(int p_id);
	long p_get_op_pot(int p_id, std::string op);
	std::vector<std::pair<std::string, int>> p_get_op_users(int p_id, std::string op);
	void p_end_poll(int p_id);
}
