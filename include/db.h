#pragma once
#include <dpp/dpp.h>
#include <string>
#include <vector>

struct sqlite3;

namespace db {
	extern sqlite3* db_ptr;

	void init();

	// aura related
	int get_aura(dpp::snowflake guild_id, dpp::snowflake user_id);
	void set_aura(dpp::snowflake guild_id, dpp::snowflake user_id, int amount);
	void add_aura(dpp::snowflake guild_id, dpp::snowflake user_id, int amount);
	void rmv_aura(dpp::snowflake guild_id, dpp::snowflake user_id, int amount);
	std::vector<std::pair<std::string, int>> get_ab(dpp::snowflake guild_id, int limit, bool bottom);
	int get_total_aura(dpp::snowflake guild_id);

	// settings related
	void set_setting(dpp::snowflake guild_id, const std::string& key, const std::string& val);
	void set_setting(dpp::snowflake guild_id, const std::string& key, int val);
	void set_setting(dpp::snowflake guild_id, const std::string& key, bool val);
	std::string get_setting_str(dpp::snowflake guild_id, const std::string&key, std::string default_val = "kai");
	int get_setting_int(dpp::snowflake guild_id, const std::string& key, int default_val = 7);
	bool get_setting_bool(dpp::snowflake guild_id, const std::string&key, bool default_val = false);

	// polls related
	struct Poll {
		int p_id;
		std::string title;
		std::string ops;
	};

	std::vector<Poll> p_get_polls(dpp::snowflake guild_id);
	Poll p_get_poll(int p_id);
	int p_set_poll(dpp::snowflake guild_id, std::string title, std::string ops);
	bool p_get_poll_end(int p_id);
	std::string p_get_poll_ops(int p_id);
	void p_place_bet(int p_id, dpp::snowflake u_id, std::string op, int amt);
	long p_get_all_pot(int p_id);
	long p_get_op_pot(int p_id, std::string op);
	std::vector<std::pair<std::string, int>> p_get_op_users(int p_id, std::string op);
	void p_end_poll(int p_id);

	// duels related
	void d_issue(dpp::snowflake guild_id, dpp::snowflake challenger, dpp::snowflake target, int bet);
	int d_check(dpp::snowflake guild_id, dpp::snowflake challenger, dpp::snowflake target);
	void d_delete(dpp::snowflake guild_id, dpp::snowflake challenger);
	bool d_outgoing(dpp::snowflake guild_id, dpp::snowflake challenger);
	long d_time(dpp::snowflake guild_id, dpp::snowflake challenger);

	// xp related
	int xp_get(dpp::snowflake guild_id, dpp::snowflake user_id);
	int lvl_get(dpp::snowflake guild_id, dpp::snowflake user_id);
	void xp_add(dpp::snowflake guild_id, dpp::snowflake user_id, int amt);
	void xp_lvl_set(dpp::snowflake guild_id, dpp::snowflake user_id, int xp, int lvl);
	long xp_time_get(dpp::snowflake guild_id, dpp::snowflake user_id);
	void xp_time_set(dpp::snowflake guild_id, dpp::snowflake user_id, long time);
	std::vector<std::pair<dpp::snowflake, int>> xp_top(dpp::snowflake guild_id, int limit);

	// vc related
	long vc_get(dpp::snowflake guild_id, dpp::snowflake user_id);
	void vc_set(dpp::snowflake guild_id, dpp::snowflake user_id, long time);
	void vc_clr(dpp::snowflake guild_id, dpp::snowflake user_id);
}
