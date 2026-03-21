#include "db.h"
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>

namespace db {
	sqlite3* db_ptr = nullptr;

	void init() {
		const char* db_path_env = std::getenv("DATABASE_PATH");
		std::string db_path = db_path_env ? db_path_env : "database.sqlite";
		if (sqlite3_open(db_path.c_str(), &db_ptr) != SQLITE_OK) {
			return;
		}

		sqlite3_exec(db_ptr, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

		const char* sql = 
			"CREATE TABLE IF NOT EXISTS users ("
			"user_id TEXT PRIMARY KEY, "
			"aura INTEGER DEFAULT 0"
			");";

		const char* setting_sql =
			"CREATE TABLE IF NOT EXISTS setting ("
			"key TEXT PRIMARY KEY, "
			"value TEXT" 
			");";	

		sqlite3_exec(db_ptr, sql, nullptr, nullptr, nullptr);
		sqlite3_exec(db_ptr, setting_sql, nullptr, nullptr, nullptr);
	}

	int get_aura(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);
		const char* sql = "SELECT aura FROM users WHERE user_id = ?;";
		sqlite3_stmt* stmt;
		int aura = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				aura = sqlite3_column_int(stmt, 0);
			}
		}
		sqlite3_finalize(stmt); 
		return aura;
	}

	std::vector<std::pair<std::string, int>> get_ab(bool bottom) {
		std::vector<std::pair<std::string, int>> results;
		std::string order = bottom ? "ASC" : "DESC";
		std::string sql = "SELECT user_id, aura FROM users ORDER BY aura " + order + " LIMIT 10;";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
				int aura = sqlite3_column_int(stmt, 1);
				results.push_back({id, aura});
			}
		}
		sqlite3_finalize(stmt);
		return results;
	}

	void add_aura(dpp::snowflake user_id, int amount) {
		std::string id_str = std::to_string(user_id);
		const char* sql = 
			"INSERT INTO users (user_id, aura) VALUES (?, ?) "
			"ON CONFLICT(user_id) DO UPDATE SET aura = aura + ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, amount);
			sqlite3_bind_int(stmt, 3, amount);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void rmv_aura(dpp::snowflake user_id, int amount) {
		add_aura(user_id, -amount);
	}

	int get_total_aura() {
		const char* sql =
			"SELECT SUM(aura) FROM users;";
		sqlite3_stmt* stmt;
		int total = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				total = sqlite3_column_int(stmt, 0);
			}
		}
		sqlite3_finalize(stmt);
		return total;
	}

	void set_aura(dpp::snowflake user_id, int amount) {
		std::string id_str = std::to_string(user_id);
		const char* sql =
			"INSERT INTO users (user_id, aura) VALUES (?, ?) "
			"ON CONFLICT(user_id) DO UPDATE SET aura = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, amount);
			sqlite3_bind_int(stmt, 3, amount);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt); 
	}
	
	void set_setting(const std::string& key, const std::string& val) {
		const char* sql = "INSERT INTO setting (key, value) VALUES (?, ?) "
				  "ON CONFLICT(key) DO UPDATE SET value = ?;";
		sqlite3_stmt* stmt;
		
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, key.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt); 
	}
	
	void set_setting(const std::string& key, int val) {
		set_setting(key, std::to_string(val));
	}

	void set_setting(const std::string& key, bool val) {
		set_setting(key, val ? "true" : "false");
	}

	std::string get_setting_str(const std::string&key, std::string default_val) {
		const char* sql = "SELECT value FROM settings WHERE key = ?;";
		sqlite3_stmt* stmt;
		std::string result = default_val;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* text = sqlite3_column_text(stmt, 0);
				if (text) {
					result = reinterpret_cast<const char*>(text);
				}
			}
		}	
		sqlite3_finalize(stmt);
		return result;
	}

	int get_setting_int(const std::string&key, int default_val) {
		std::string val = get_setting_str(key);
		return val.empty() ? default_val : std::stoi(val);
	}

	bool get_setting_bool(const std::string& key, bool default_val) {
		std::string val = get_setting_str(key);
		return (val == "true");
	}
}
