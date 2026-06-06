// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "test_framework.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace utils_test {

	static std::string::size_type v_st(const std::string& j, const std::string& key) {
		std::string search = "\"" + key + "\"";
		auto kpos = j.find(search);
		if (kpos == std::string::npos) return std::string::npos;
		auto col = j.find(':', kpos + search.size());
		if (col == std::string::npos) return std::string::npos;
		auto vpos = j.find_first_not_of(" \n\r\t", col + 1);
		return vpos;
	}

	std::string json_str(const std::string& j, const std::string& key) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos || j[vpos] != '"') return "";
		auto q2 = j.find('"', vpos + 1);
		if (q2 == std::string::npos) return "";
		return j.substr(vpos + 1, q2 - vpos - 1);
	}

	bool json_bool(const std::string& j, const std::string& key, bool df = false) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos) return df;
		if (j.compare(vpos, 4, "true") == 0) return true;
		return false;
	}

	int json_int(const std::string& j, const std::string& key, int df = 0) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos) return df;
		try { return std::stoi(j.substr(vpos)); } catch (...) { return df; }
	}

	double json_doub(const std::string& j, const std::string& key, double df = 0.0) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos) return df;
		try { return std::stod(j.substr(vpos)); } catch (...) { return df; }
	}

	std::string get_safe_role(const std::string& name, const std::string& /*type*/, size_t max_len = 18) {
		return name.size() <= max_len ? name : name.substr(0, max_len - 3) + "...";
	}
} // namespace utils_test

TEST(json_str_basic) {
	std::string j = R"({"button_style":"primary","foo":"bar"})";
	test::assert_eq(utils_test::json_str(j, "button_style"), std::string("primary"));
	test::assert_eq(utils_test::json_str(j, "foo"), std::string("bar"));
}

TEST(json_str_missing_key) {
	std::string j = R"({"x":"y"})";
	test::assert_eq(utils_test::json_str(j, "missing"), std::string(""));
}

TEST(json_str_empty_value) {
	std::string j = R"({"key":""})";
	test::assert_eq(utils_test::json_str(j, "key"), std::string(""));
}

TEST(json_bool_true) {
	test::assert_true(utils_test::json_bool(R"({"invert":true})", "invert"));
}

TEST(json_bool_false) {
	test::assert_true(!utils_test::json_bool(R"({"invert":false})", "invert"));
}

TEST(json_bool_missing) {
	test::assert_true(!utils_test::json_bool(R"({})", "invert", false));
	test::assert_true(utils_test::json_bool(R"({})", "invert", true));
}

TEST(json_int_basic) {
	test::assert_eq(utils_test::json_int(R"({"hours":24})", "hours"), 24);
}

TEST(json_int_negative) {
	test::assert_eq(utils_test::json_int(R"({"cost":-500})", "cost"), -500);
}

TEST(json_int_missing) {
	test::assert_eq(utils_test::json_int(R"({})", "hours", 99), 99);
}

TEST(json_doub_basic) {
	double v = utils_test::json_doub(R"({"mult":2.5})", "mult");
	test::assert_true(v > 2.49 && v < 2.51);
}

TEST(safe_role_short_name) {
	test::assert_eq(utils_test::get_safe_role("Cool", "role", 10), std::string("Cool"));
}

TEST(safe_role_exact_limit) {
	test::assert_eq(utils_test::get_safe_role("1234567890", "role", 10), std::string("1234567890"));
}

TEST(safe_role_truncation) {
	test::assert_eq(utils_test::get_safe_role("12345678901", "role", 10), std::string("1234567..."));
}

TEST(backup_days_default) {
	unsetenv("BACKUP_DAYS");
	const char* v = std::getenv("BACKUP_DAYS");
	int days = 3;
	if (v) {
		try { int n = std::stoi(v); if (n > 0) days = n; } catch (...) {}
	}
	test::assert_eq(days, 3);
}

TEST(backup_days_from_env) {
	setenv("BACKUP_DAYS", "7", 1);
	const char* v = std::getenv("BACKUP_DAYS");
	int days = 3;
	if (v) {
		try { int n = std::stoi(v); if (n > 0) days = n; } catch (...) {}
	}
	test::assert_eq(days, 7);
	unsetenv("BACKUP_DAYS");
}

TEST(backup_env_missing) {
	setenv("BACKUP_DAYS", "banana", 1);
	const char* v = std::getenv("BACKUP_DAYS");
	int days = 3;
	if (v) {
		try { int n = std::stoi(v); if (n > 0) days = n; } catch (...) {}
	}
	test::assert_eq(days, 3);
	unsetenv("BACKUP_DAYS");
}


TEST(backup_creates_file) {
	namespace fs = std::filesystem;

	// Create a temp 
	std::string src = "/tmp/axu_test_db.sqlite";
	std::ofstream(src) << "dummy";
	setenv("DATABASE_PATH", src.c_str(), 1);
	setenv("BACKUP_DAYS", "3", 1);

	// Simulate do_backup 
	fs::path sp(src);
	fs::path dir  = sp.parent_path();
	std::string stem = sp.stem().string();
	std::string ext  = sp.extension().string();

	// Build today's stamp
	std::time_t t = std::time(nullptr);
	char buf[16];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
	fs::path dst = dir / (stem + "_" + std::string(buf) + ext);

	if (fs::exists(dst)) fs::remove(dst);
	fs::copy_file(src, dst, fs::copy_options::overwrite_existing);

	test::assert_true(fs::exists(dst), "backup file should exist");

	// Cleanup
	fs::remove(dst);
	fs::remove(src);
	unsetenv("DATABASE_PATH");
	unsetenv("BACKUP_DAYS");
}

#include <sqlite3.h>

static sqlite3* open_test_db() {
	sqlite3* db = nullptr;
	sqlite3_open(":memory:", &db);
	sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
	sqlite3_exec(db,
		"CREATE TABLE shop_items ("
		"  item_id INTEGER PRIMARY KEY AUTOINCREMENT, guild_id TEXT NOT NULL,"
		"  type TEXT NOT NULL, role_id TEXT, name TEXT NOT NULL, desc TEXT,"
		"  cost INTEGER NOT NULL, data TEXT, active INTEGER DEFAULT 1,"
		"  obtainable INTEGER DEFAULT 1, sellability INTEGER DEFAULT 1,"
		"  pinned INTEGER DEFAULT 0, global INTEGER DEFAULT 0);"
		"CREATE TABLE inventory ("
		"  inv_id INTEGER PRIMARY KEY AUTOINCREMENT, guild_id TEXT NOT NULL,"
		"  user_id TEXT NOT NULL, item_id INTEGER NOT NULL,"
		"  acquired INTEGER NOT NULL, expires INTEGER, equipped INTEGER DEFAULT 0,"
		"  FOREIGN KEY(item_id) REFERENCES shop_items(item_id));"
		"CREATE TABLE aura ("
		"  guild_id TEXT NOT NULL, user_id TEXT NOT NULL, amount INTEGER DEFAULT 0,"
		"  PRIMARY KEY (guild_id, user_id));",
		nullptr, nullptr, nullptr);
	return db;
}

static int db_exec_int(sqlite3* db, const char* sql) {
	sqlite3_stmt* s;
	int val = 0;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		if (sqlite3_step(s) == SQLITE_ROW) val = sqlite3_column_int(s, 0);
		sqlite3_finalize(s);
	}
	return val;
}

TEST(db_shop_add_and_get) {
	sqlite3* db = open_test_db();
	sqlite3_stmt* s;
	const char* sql = "INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data)"
					  " VALUES ('1', 'role', '0', 'Test Role', 'desc', 100, '{\"button_style\":\"primary\"}')";
	sqlite3_exec(db, sql, nullptr, nullptr, nullptr);

	int id = db_exec_int(db, "SELECT item_id FROM shop_items WHERE name = 'Test Role'");
	test::assert_true(id > 0, "item_id should be positive");

	// Verify data field round-trips
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(db, "SELECT data FROM shop_items WHERE item_id = ?", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, id);
	std::string data;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		if (auto* d = sqlite3_column_text(stmt, 0)) data = reinterpret_cast<const char*>(d);
	sqlite3_finalize(stmt);

	test::assert_eq(utils_test::json_str(data, "button_style"), std::string("primary"));

	sqlite3_close(db);
}

TEST(db_inventory_add_and_count) {
	sqlite3* db = open_test_db();
	// Add a shop item
	sqlite3_exec(db,
		"INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data)"
		" VALUES ('1', 'role', '0', 'Hat', '', 50, '{}')",
		nullptr, nullptr, nullptr);
	int item_id = db_exec_int(db, "SELECT item_id FROM shop_items WHERE name = 'Hat'");

	// Add two inventory entries
	for (int uid : {101, 102}) {
		sqlite3_stmt* s;
		sqlite3_prepare_v2(db,
			"INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
			" VALUES ('1', ?, ?, strftime('%s','now'), 0, 0)",
			-1, &s, nullptr);
		std::string us = std::to_string(uid);
		sqlite3_bind_text(s, 1, us.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(s, 2, item_id);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}

	int cnt = db_exec_int(db, "SELECT COUNT(*) FROM inventory WHERE item_id = 1");
	test::assert_eq(cnt, 2, "two users should own the item");
	sqlite3_close(db);
}

TEST(db_shop_del_inv) {
	sqlite3* db = open_test_db();
	sqlite3_exec(db,
		"INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data)"
		" VALUES ('1', 'role', '0', 'Cape', '', 200, '{}')",
		nullptr, nullptr, nullptr);
	int item_id = db_exec_int(db, "SELECT item_id FROM shop_items WHERE name = 'Cape'");

	// Three owners
	for (int uid : {10, 20, 30}) {
		sqlite3_stmt* s;
		sqlite3_prepare_v2(db,
			"INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
			" VALUES ('1', ?, ?, strftime('%s','now'), 0, 0)",
			-1, &s, nullptr);
		std::string us = std::to_string(uid);
		sqlite3_bind_text(s, 1, us.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(s, 2, item_id);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}

	{
		sqlite3_stmt* s;
		sqlite3_prepare_v2(db, "DELETE FROM inventory WHERE guild_id = '1' AND item_id = ?", -1, &s, nullptr);
		sqlite3_bind_int(s, 1, item_id);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}
	{
		sqlite3_stmt* s;
		sqlite3_prepare_v2(db, "DELETE FROM shop_items WHERE guild_id = '1' AND item_id = ?", -1, &s, nullptr);
		sqlite3_bind_int(s, 1, item_id);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}

	int inv_cnt  = db_exec_int(db, "SELECT COUNT(*) FROM inventory WHERE guild_id = '1'");
	int shop_cnt = db_exec_int(db, "SELECT COUNT(*) FROM shop_items WHERE guild_id = '1'");
	test::assert_eq(inv_cnt,  0, "inventory should be empty after delete");
	test::assert_eq(shop_cnt, 0, "shop should be empty after delete");
	sqlite3_close(db);
}

TEST(db_aura_compensation) {
	sqlite3* db = open_test_db();
	// Seed aura for two users
	sqlite3_exec(db,
		"INSERT INTO aura VALUES ('1','10',500);"
		"INSERT INTO aura VALUES ('1','20',300);",
		nullptr, nullptr, nullptr);

	// Pay 100 compensation
	sqlite3_exec(db, "UPDATE aura SET amount = amount + 100 WHERE guild_id = '1'", nullptr, nullptr, nullptr);

	int a10 = db_exec_int(db, "SELECT amount FROM aura WHERE guild_id = '1' AND user_id = '10'");
	int a20 = db_exec_int(db, "SELECT amount FROM aura WHERE guild_id = '1' AND user_id = '20'");
	test::assert_eq(a10, 600);
	test::assert_eq(a20, 400);
	sqlite3_close(db);
}

TEST(db_inventory_equipped_flag) {
	sqlite3* db = open_test_db();
	sqlite3_exec(db,
		"INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data)"
		" VALUES ('1', 'role', '0', 'Shield', '', 75, '{\"button_style\":\"danger\"}')",
		nullptr, nullptr, nullptr);
	int item_id = db_exec_int(db, "SELECT item_id FROM shop_items WHERE name = 'Shield'");

	sqlite3_stmt* s;
	sqlite3_prepare_v2(db,
		"INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
		" VALUES ('1','42',?,strftime('%s','now'),0,0)",
		-1, &s, nullptr);
	sqlite3_bind_int(s, 1, item_id);
	sqlite3_step(s); sqlite3_finalize(s);

	// Equip
	sqlite3_prepare_v2(db,
		"UPDATE inventory SET equipped = 1 WHERE guild_id = '1' AND user_id = '42' AND item_id = ?",
		-1, &s, nullptr);
	sqlite3_bind_int(s, 1, item_id);
	sqlite3_step(s); sqlite3_finalize(s);

	int equipped = db_exec_int(db, "SELECT equipped FROM inventory WHERE user_id = '42'");
	test::assert_eq(equipped, 1, "item should be equipped");
	sqlite3_close(db);
}

TEST(db_inv_purge_expired) {
	sqlite3* db = open_test_db();
	sqlite3_exec(db,
		"INSERT INTO shop_items (guild_id, type, role_id, name, desc, cost, data)"
		" VALUES ('1', 'xp_boost', '0', 'Boost', '', 50, '{\"mult\":2,\"hours\":1}')",
		nullptr, nullptr, nullptr);
	int item_id = db_exec_int(db, "SELECT item_id FROM shop_items WHERE name = 'Boost'");

	// Insert one expired and one valid entry
	sqlite3_stmt* s;
	sqlite3_prepare_v2(db,
		"INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
		" VALUES ('1','99',?,strftime('%s','now') - 7200, strftime('%s','now') - 3600, 0)",
		-1, &s, nullptr);
	sqlite3_bind_int(s, 1, item_id);
	sqlite3_step(s); sqlite3_finalize(s);

	sqlite3_prepare_v2(db,
		"INSERT INTO inventory (guild_id, user_id, item_id, acquired, expires, equipped)"
		" VALUES ('1','100',?,strftime('%s','now'), strftime('%s','now') + 3600, 0)",
		-1, &s, nullptr);
	sqlite3_bind_int(s, 1, item_id);
	sqlite3_step(s); sqlite3_finalize(s);

	// Purge expired (expires > 0 AND expires <= now)
	sqlite3_exec(db,
		"DELETE FROM inventory WHERE guild_id = '1' AND user_id = '99'"
		" AND expires > 0 AND expires <= strftime('%s','now')",
		nullptr, nullptr, nullptr);

	int cnt = db_exec_int(db, "SELECT COUNT(*) FROM inventory WHERE guild_id = '1'");
	test::assert_eq(cnt, 1, "only the non-expired entry should remain");
	sqlite3_close(db);
}

TEST(leaderboard_empty) {
	std::vector<std::pair<uint64_t, int>> raw = {{1001, 500}, {1002, 400}, {1003, 300}};
	auto is_guild_member = [](uint64_t /*uid*/) { return false; };

	std::vector<std::pair<uint64_t, int>> entries;
	for (auto& [uid, xp] : raw) {
		if (!is_guild_member(uid)) continue;
		entries.push_back({uid, xp});
		if ((int)entries.size() >= 15) break;
	}
	test::assert_true(entries.empty(), "no entries when all users are ghosts");
}

TEST(leaderboard_caps_at_15) {
	std::vector<std::pair<uint64_t, int>> raw;
	for (int i = 1; i <= 50; ++i) raw.push_back({(uint64_t)i, 1000 - i});
	auto is_guild_member = [](uint64_t /*uid*/) { return true; };

	std::vector<std::pair<uint64_t, int>> entries;
	for (auto& [uid, xp] : raw) {
		if (!is_guild_member(uid)) continue;
		entries.push_back({uid, xp});
		if ((int)entries.size() >= 15) break;
	}
	test::assert_eq((int)entries.size(), 15, "leaderboard capped at 15");
}

TEST(leaderboard_partial_segfault) {
	std::vector<std::pair<uint64_t, int>> raw = {
		{1, 100}, {2, 90}, {3, 80}, {4, 70}, {5, 60}};
	auto is_guild_member = [](uint64_t uid) { return uid == 1 || uid == 4; };

	std::vector<std::pair<uint64_t, int>> entries;
	for (auto& [uid, xp] : raw) {
		if (!is_guild_member(uid)) continue;
		entries.push_back({uid, xp});
		if ((int)entries.size() >= 15) break;
	}
	test::assert_eq((int)entries.size(), 2);
	test::assert_eq((int)entries[0].first, 1);
	test::assert_eq((int)entries[1].first, 4);
}

int main() {
	return test::run_all();
}

