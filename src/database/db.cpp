#include "db.h"
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

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
			"aura INTEGER DEFAULT 0, "
			"xp INTEGER DEFAULT 0, "
			"level INTEGER DEFAULT 0, "
			"xp_time INTEGER DEFAULT 0, "
			"migrated INTEGER DEFAULT 0"
			");";

		const char* setting_sql =
			"CREATE TABLE IF NOT EXISTS settings ("
			"key TEXT PRIMARY KEY, "
			"value TEXT" 
			");";	

		const char* poll_sql = 
			"CREATE TABLE IF NOT EXISTS polls ("
			"p_id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"title TEXT, "
			"ops TEXT, "
			"active INTEGER DEFAULT 1"
			");";

		const char* bets_sql =
			"CREATE TABLE IF NOT EXISTS bets ("
			"p_id INTEGER, "
			"u_id TEXT, "
			"op TEXT, "
			"amt INTEGER, "
			"FOREIGN KEY(p_id) REFERENCES polls(p_id)"
			");";

		const char* duels_sql =
			"CREATE TABLE IF NOT EXISTS duels ("
			"challenger TEXT PRIMARY KEY, "
			"target TEXT, "
			"bet INTEGER, "
			"issue_time INTEGER"
			");";

		// this is fucking stupid. TODO: find a way to not need to do this as a separate table but its not striking my brain rn with any easy solution that doesnt break shit.
		const char* vc_sql = 
			"CREATE TABLE IF NOT EXISTS voice ("
			"user_id TEXT PRIMARY KEY, "
			"join_time INTEGER"
			");";


		sqlite3_exec(db_ptr, sql, nullptr, nullptr, nullptr);
		sqlite3_exec(db_ptr, setting_sql, nullptr, nullptr, nullptr);
		sqlite3_exec(db_ptr, poll_sql, nullptr, nullptr, nullptr);
		sqlite3_exec(db_ptr, bets_sql, nullptr, nullptr, nullptr);
		sqlite3_exec(db_ptr, duels_sql, nullptr, nullptr, nullptr);
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

	std::vector<std::pair<std::string, int>> get_ab(int limit, bool bottom) {
		std::vector<std::pair<std::string, int>> results;
		std::string order = bottom ? "ASC" : "DESC";
		std::string sql = "SELECT user_id, aura FROM users ORDER BY aura " + order + " LIMIT " + std::to_string(limit) + ";";

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
		const char* sql = "INSERT INTO settings (key, value) VALUES (?, ?) "
				  "ON CONFLICT(key) DO UPDATE SET value = ?;";
		sqlite3_stmt* stmt;
		
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, val.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, val.c_str(), -1, SQLITE_TRANSIENT);
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
		std::string val = get_setting_str(key, std::to_string(default_val));
		return val.empty() ? default_val : std::stoi(val);
	}

	bool get_setting_bool(const std::string& key, bool default_val) {
		std::string val = get_setting_str(key, "false");
		return (val == "true");
	}

	std::vector<Poll> p_get_polls() {
		std::vector<Poll> list;
		const char* sql = "SELECT p_id, title, ops FROM polls WHERE active = 1;";
		sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
				Poll p;
                p.p_id = sqlite3_column_int(stmt, 0);
                p.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                p.ops = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        		list.push_back(p);
            }
        }
        sqlite3_finalize(stmt);
        return list;
	}

	Poll p_get_poll(int p_id) {
		const char* sql = "SELECT p_id, title, ops FROM polls WHERE p_id = ?;";
        sqlite3_stmt* stmt;
        Poll p = {-1, "", ""};
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, p_id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                p.p_id = sqlite3_column_int(stmt, 0);
                const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char* opts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                p.title = title ? title : "";
                p.ops = opts ? opts : "";
            }
        }
        sqlite3_finalize(stmt);
        return p;
	}

	int p_set_poll(std::string title, std::string ops) {
		const char* sql = "INSERT INTO polls (title, ops) VALUES (?, ?);";
		sqlite3_stmt* stmt;
		int id = -1;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, ops.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
			id = (int)sqlite3_last_insert_rowid(db_ptr);
		}
		sqlite3_finalize(stmt);
		return id;
	}

	std::string p_get_poll_ops(int p_id) {
		const char* sql = "SELECT ops FROM polls WHERE p_id = ? AND active = 1;";
		sqlite3_stmt* stmt;
		std::string ops = "";
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				const unsigned char* text = sqlite3_column_text(stmt, 0);
				if (text) {
					ops = reinterpret_cast<const char*>(text);
				}
			}
		}
		sqlite3_finalize(stmt);
		return ops;
	}

	bool p_get_poll_end(int p_id) {
		const char* sql = "SELECT active FROM polls WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		bool active = false;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				active = (sqlite3_column_int(stmt, 0) == 1);
			}
		}
		sqlite3_finalize(stmt);
		return active;
	}

	void p_end_poll(int p_id) {
		const char* sql = "UPDATE polls SET active = 0 WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void p_place_bet(int p_id, dpp::snowflake user_id, std::string op, int amt) {
		std::string s_uid = std::to_string(user_id);
		const char* sql = "INSERT INTO bets (p_id, u_id, op, amt) VALUES (?, ?, ?, ?);"; 
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_bind_text(stmt, 2, s_uid.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, op.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 4, amt);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	long p_get_all_pot(int p_id) {
		const char* sql = "SELECT SUM(amt) FROM bets WHERE p_id = ?;";
		sqlite3_stmt* stmt;
		long total = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				total = sqlite3_column_int(stmt, 0);
			}
		}
		sqlite3_finalize(stmt);
		return total;
	}

	long p_get_op_pot(int p_id, std::string op) {
		const char* sql = "SELECT SUM(amt) FROM bets WHERE p_id = ? AND op = ?;";
		sqlite3_stmt* stmt;
		long total = 0;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_bind_text(stmt, 2, op.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				total = sqlite3_column_int(stmt, 0);
			}
		}
		sqlite3_finalize(stmt);
		return total;
	}

	std::vector<std::pair<std::string, int>> p_get_op_users(int p_id, std::string op) {
		std::vector<std::pair<std::string, int>> winners;
		const char* sql = "SELECT u_id, amt FROM bets WHERE p_id = ? AND op = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, p_id);
			sqlite3_bind_text(stmt, 2, op.c_str(), -1, SQLITE_TRANSIENT);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				const char* u_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
				int amt = sqlite3_column_int(stmt, 1);
				winners.push_back({u_id, amt});
			}
		}
		sqlite3_finalize(stmt);
		return winners;
	}

	void d_issue(dpp::snowflake challenger, dpp::snowflake target, int bet) {
		std::string c_str = std::to_string(challenger);
		std::string t_str = std::to_string(target);
		long issue_time = std::time(nullptr);
		const char* sql = "INSERT INTO duels (challenger, target, bet, issue_time) VALUES (?, ?, ?, ?) "
							"ON CONFLICT(challenger) DO UPDATE SET target = ?, bet = ?, issue_time = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, c_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, t_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, bet);
            sqlite3_bind_int64(stmt, 4, issue_time);
            sqlite3_bind_text(stmt, 5, t_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 6, bet);
            sqlite3_bind_int64(stmt, 7, issue_time);
            sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	int d_check(dpp::snowflake challenger, dpp::snowflake target) {
		std::string c_str = std::to_string(challenger);
		std::string t_str = std::to_string(target);
		const char* sql = "SELECT bet, issue_time FROM duels WHERE challenger = ? AND target = ?;";
		sqlite3_stmt* stmt;
		int bet = -1;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, c_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, t_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				bet = sqlite3_column_int(stmt, 0);
				long issue_time = sqlite3_column_int64(stmt, 1);
				if (std::time(nullptr) - issue_time >= 120) {
					bet = -1;
				}
			}
		}
		sqlite3_finalize(stmt);
		if (bet == -1) {
			d_delete(challenger);
		}
		return bet;
	}

	long d_time(dpp::snowflake challenger) {
		std::string c_str = std::to_string(challenger);
		const char* sql = "SELECT issue_time FROM duels WHERE challenger = ?;";
		sqlite3_stmt* stmt;
		long time = -1;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, c_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				time = sqlite3_column_int64(stmt, 0);
			}
		}
		sqlite3_finalize(stmt);
		return time;
	}

	void d_delete(dpp::snowflake challenger) {
		std::string c_str = std::to_string(challenger);
		const char* sql = "DELETE FROM duels WHERE challenger = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, c_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	bool d_outgoing(dpp::snowflake challenger) {
		std::string c_str = std::to_string(challenger);
		const char* sql = "SELECT 1 FROM duels WHERE challenger = ?;";
		sqlite3_stmt* stmt;
		bool yes = false;

		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, c_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				yes = true;
			}
		}
		sqlite3_finalize(stmt);
		return yes;
	}

	int xp_get(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);
		const char* sql = "SELECT xp FROM users WHERE user_id = ?;";
		sqlite3_stmt* stmt;
		int xp = 0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) xp = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return xp;
	}

	int lvl_get(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "SELECT level FROM users WHERE user_id = ?;";
        sqlite3_stmt* stmt;
        int lvl = 0;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) lvl = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt); 
        return lvl;
	}

	void xp_add(dpp::snowflake user_id, int amt) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "INSERT INTO users (user_id, xp) VALUES (?, ?) ON CONFLICT(user_id) DO UPDATE SET xp = xp + ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, amt);
            sqlite3_bind_int(stmt, 3, amt);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
	}

	void xp_lvl_set(dpp::snowflake user_id, int xp, int lvl) {
        std::string id_str = std::to_string(user_id);
        const char* sql = "INSERT INTO users (user_id, xp, level) VALUES (?, ?, ?) ON CONFLICT(user_id) DO UPDATE SET xp = ?, level = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, xp);
            sqlite3_bind_int(stmt, 3, lvl);
            sqlite3_bind_int(stmt, 4, xp);
            sqlite3_bind_int(stmt, 5, lvl);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }

	long xp_time_get(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "SELECT xp_time FROM users WHERE user_id = ?;";
        sqlite3_stmt* stmt;
        long time = 0;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) time = sqlite3_column_int64(stmt, 0);
        }
        sqlite3_finalize(stmt); 
        return time;
	}

	void xp_time_set(dpp::snowflake user_id, long time) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "INSERT INTO users (user_id, xp_time) VALUES (?, ?) ON CONFLICT(user_id) DO UPDATE SET xp_time = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 2, time);
            sqlite3_bind_int64(stmt, 3, time);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
	}

	bool xp_migrate_get(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);
		const char* sql = "SELECT migrated FROM users WHERE user_id = ?;";
        sqlite3_stmt* stmt;
        bool mig = false;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) mig = (sqlite3_column_int(stmt, 0) == 1);
        }
        sqlite3_finalize(stmt);
        return mig;
	}

	void xp_migrate_set(dpp::snowflake user_id, bool stat) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "INSERT INTO users (user_id, migrated) VALUES (?, ?) ON CONFLICT(user_id) DO UPDATE SET migrated = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, stat ? 1 : 0);
            sqlite3_bind_int(stmt, 3, stat ? 1 : 0);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
	}

	std::vector<std::pair<dpp::snowflake, int>> xp_top(int limit) {
		std::vector<std::pair<dpp::snowflake, int>> result;
		const char* sql = "SELECT user_id, xp FROM users WHERE xp > 0 ORDER BY xp DESC LIMIT ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, limit);
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				dpp::snowflake uid = std::stoull(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
				int xp = sqlite3_column_int(stmt, 1);
				result.push_back({uid, xp});
			}
		}
		sqlite3_finalize(stmt);
		return result;
	}

	long vc_get(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "SELECT join_time FROM voice WHERE user_id = ?;";
        sqlite3_stmt* stmt;
		long time = 0;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW) time = sqlite3_column_int64(stmt, 0);
        }
        sqlite3_finalize(stmt);
		return time;
	}

	void vc_set(dpp::snowflake user_id, long time) {
		std::string id_str = std::to_string(user_id);
        const char* sql = "INSERT INTO voice (user_id, join_time) VALUES (?, ?) ON CONFLICT(user_id) DO UPDATE SET join_time = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(stmt, 2, time);
			sqlite3_bind_int64(stmt, 3, time);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}

	void vc_clr(dpp::snowflake user_id) {
		std::string id_str = std::to_string(user_id);

		const char* sql = "DELETE FROM voice WHERE user_id = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(stmt, 1, id_str.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
	}
}
