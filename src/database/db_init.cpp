// overhauling the entire database system into separate files so that I don't have to work with a monolith of like 50,000,000 lines every time I make any database changes.
//
// db_init.cpp: just has the init and the initial db calls

#include "db.h"
#include <sqlite3.h>
#include <iostream>
#include <vector>

namespace db {
    sqlite3* db_ptr = nullptr;

    void init() {
        const char* db_path_env = std::getenv("DATABASE_PATH");
        std::string db_path = db_path_env ? db_path_env : "database.sqlite";
        if (sqlite3_open(db_path.c_str(), &db_ptr) != SQLITE_OK) {
            std::cerr << "database failed to load: " << sqlite3_errmsg(db_ptr) << std::endl;
            return;
        }

        sqlite3_exec(db_ptr, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

        const char* db_version = 
            "CREATE TABLE IF NOT EXISTS db_version ("
            "version INTEGER PRIMARY KEY, "
            "updated INTEGER"
            ");";
        sqlite3_exec(db_ptr, db_version, nullptr, nullptr, nullptr);

        // get latest version of the database so older versions can be migrated if necessary.
        int cur_version = 0;
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, "SELECT MAX(version) FROM db_version;", -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                cur_version = sqlite3_column_int(stmt, 0);
            }
        }

        const char* v1_schema =
            "CREATE TABLE IF NOT EXISTS aura ("
            "guild_id TEXT NOT NULL, "
            "user_id TEXT NOT NULL, "
            "amount INTEGER DEFAULT 0, "
            "PRIMARY KEY (guild_id, user_id));"

            "CREATE TABLE IF NOT EXISTS xp ("
            "guild_id TEXT NOT NULL, user_id TEXT NOT NULL, xp INTEGER DEFAULT 0, "
            "level INTEGER DEFAULT 0, xp_time INTEGER DEFAULT 0, "
            "PRIMARY KEY (guild_id, user_id));"

            "CREATE TABLE IF NOT EXISTS guild_settings ("
            "guild_id TEXT NOT NULL, key TEXT NOT NULL, value TEXT, "
            "PRIMARY KEY (guild_id, key));"

            "CREATE TABLE IF NOT EXISTS polls_tmp ("
            "p_id INTEGER PRIMARY KEY AUTOINCREMENT, guild_id TEXT NOT NULL, "
            "title TEXT, ops TEXT, active INTEGER DEFAULT 1);"

            "CREATE TABLE IF NOT EXISTS bets ("
            "p_id INTEGER, u_id TEXT, op TEXT, amt INTEGER, "
            "FOREIGN KEY(p_id) REFERENCES polls(p_id));"

            "CREATE TABLE IF NOT EXISTS duels_tmp ("
            "guild_id TEXT NOT NULL, challenger TEXT NOT NULL, target TEXT, "
            "bet INTEGER, issue_time INTEGER, "
            "PRIMARY KEY (guild_id, challenger));"

            "CREATE TABLE IF NOT EXISTS voice_tmp ("
            "guild_id TEXT NOT NULL, user_id TEXT NOT NULL, join_time INTEGER, "
            "PRIMARY KEY (guild_id, user_id));";

        sqlite3_exec(db_ptr, v1_schema, nullptr, nullptr, nullptr);
    }
}
