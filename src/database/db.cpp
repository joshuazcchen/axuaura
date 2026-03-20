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
        
        char* err_msg = nullptr;
        sqlite3_exec(db_ptr, sql, nullptr, nullptr, &err_msg);
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
}
