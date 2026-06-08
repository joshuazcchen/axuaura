// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "message.h"

#include <ctime>
#include <mutex>
#include <random>
#include <unordered_map>

#include "config.h"
#include "db.h"

namespace message {

	static std::mutex cd_mtx;
	static std::unordered_map<std::string, long> aura_loss_cd;

	static bool cd_ok(dpp::snowflake g_id, dpp::snowflake u_id) {
		int cd_secs = db::get_setting_int(g_id, "auralosscd", 30);
		std::string key = std::to_string(g_id) + "_" + std::to_string(u_id);
		long now = std::time(nullptr);
		std::lock_guard<std::mutex> lk(cd_mtx);
		auto it = aura_loss_cd.find(key);
		if (it != aura_loss_cd.end() && now - it->second < cd_secs) return false;
		aura_loss_cd[key] = now;
		return true;
	}

	std::string resp_msg(const std::vector<std::string>& pool) {
		thread_local std::philox4x32 gen(std::random_device{}());
		std::uniform_int_distribution<> dis(0, pool.size() - 1);
		return pool[dis(gen)];
	}

	void do_aura_loss(dpp::cluster& bot, dpp::snowflake g_id, dpp::snowflake u_id, dpp::snowflake ch_id,
					  dpp::snowflake msg_id, bool is_non_eng) {
		if (!cd_ok(g_id, u_id)) return;
		db::rmv_aura(g_id, u_id, db::get_setting_int(g_id, "auralossamt", 100));
		std::string text = is_non_eng ? resp_msg(config::SPANISH_LOSS) : resp_msg(config::AURA_LOSSES);
		dpp::message rep(ch_id, text);
		rep.set_reference(msg_id);
		bot.message_create(rep);
	}

} // namespace message
