#include "db.h"
#include <iostream>
#include <ctime>
#include <sqlite3.h>

namespace db {
    void inv_add(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
        sqlite3_stmt* s;
        std::string sql = "INSERT INTO inventory (guild_id, user_id, item_id, acquired, equipped) VALUES (?, ?, ?, ?, 0)";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 3, item_id);
            sqlite3_bind_int64(s, 4, std::time(nullptr));
            sqlite3_step(s); sqlite3_finalize(s);
        }
    }

    void inv_rm(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
        sqlite3_stmt* s;
        std::string sql = "DELETE FROM inventory WHERE guild_id = ? AND user_id = ? AND item_id = ?";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 3, item_id);
            sqlite3_step(s); sqlite3_finalize(s);
        }
    }

    bool inv_has(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
        sqlite3_stmt* s;
        bool has = false;
        std::string sql = "SELECT 1 FROM inventory WHERE guild_id = ? AND user_id = ? AND item_id = ? LIMIT 1";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 3, item_id);
            if (sqlite3_step(s) == SQLITE_ROW) has = true;
            sqlite3_finalize(s);
        }
        return has;
    }

    std::vector<dpp::snowflake> inv_list(dpp::snowflake g_id, int item_id) {
        sqlite3_stmt* s;
        std::vector<dpp::snowflake> users;
        std::string sql = "SELECT user_id FROM inventory WHERE guild_id = ? AND item_id = ?";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 2, item_id);

            while (sqlite3_step(s) == SQLITE_ROW) {
                const char* u_id_str = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
                if (u_id_str) {
                    users.push_back(std::stoull(u_id_str));
                }
            }
            sqlite3_finalize(s);
        }
        return users;
    }

    std::vector<InvItem> inv_get_user(dpp::snowflake g_id, dpp::snowflake u_id) {
        std::vector<InvItem> items;
        sqlite3_stmt* s;
        std::string sql = "SELECT i.inv_id, s.item_id, s.name, s.type, s.role_id, i.equipped, i.acquired FROM inventory i JOIN shop_items s ON i.item_id = s.item_id WHERE i.guild_id = ? AND i.user_id = ?";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(s) == SQLITE_ROW) {
                InvItem i;
                i.inv_id = sqlite3_column_int(s, 0);
                i.item_id = sqlite3_column_int(s, 1);
                i.name = reinterpret_cast<const char*>(sqlite3_column_text(s, 2));
                i.type = reinterpret_cast<const char*>(sqlite3_column_text(s, 3));
                i.role_id = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(s, 4)));
                i.equipped = sqlite3_column_int(s, 5) == 1;
                i.acquired = sqlite3_column_int64(s, 6);
                items.push_back(i);
            }
            sqlite3_finalize(s);
        }
        return items;
    }

    void inv_eq(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
        sqlite3_stmt* s;
        std::string sql = "UPDATE inventory SET equipped = 1 WHERE guild_id = ? AND user_id = ? AND item_id = ?";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 3, item_id);
            sqlite3_step(s); sqlite3_finalize(s);
        }
    }

    void inv_uneq(dpp::snowflake g_id, dpp::snowflake u_id, int item_id) {
        sqlite3_stmt* s;
        std::string sql = "UPDATE inventory SET equipped = 0 WHERE guild_id = ? AND user_id = ? AND item_id = ?";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(s, 3, item_id);
            sqlite3_step(s); sqlite3_finalize(s);
        }
    }

    double inv_xp_mult(dpp::snowflake g_id, dpp::snowflake u_id) {
        sqlite3_stmt* s;
        double max_mult = 1.0;
        std::string sql = "SELECT s.data FROM inventory i JOIN shop_items s ON i.item_id = s.item_id WHERE i.guild_id = ? AND i.user_id = ? AND i.equipped = 1 AND s.type = 'xp_boost'";
        if (sqlite3_prepare_v2(db_ptr, sql.c_str(), -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(s, 1, std::to_string(g_id).c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, std::to_string(u_id).c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(s) == SQLITE_ROW) {
                const unsigned char* data = sqlite3_column_text(s, 0);
                if (data) {
                    std::string raw_val = reinterpret_cast<const char*>(data);
                    if (!raw_val.empty()) {
                        double m = std::stod(raw_val);
                        if (m > max_mult) max_mult = m;
                    }
                }
            }
            sqlite3_finalize(s);
        }
        return max_mult;
    }
}
