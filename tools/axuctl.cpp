#include <ctime>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "dotenv.h"

static sqlite3* db;

static void db_open() {
	dotenv::init();
	const char* path = std::getenv("DATABASE_PATH");
	if (!path) path = "database.sqlite";
	if (sqlite3_open(path, &db) != SQLITE_OK) {
		std::cerr << "failed to open db: " << sqlite3_errmsg(db) << "\n";
		std::exit(1);
	}
	sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
	sqlite3_exec(db,
	 "CREATE TABLE IF NOT EXISTS user_badges ("
	 "user_id TEXT NOT NULL, badge_id TEXT NOT NULL, granted_at INTEGER NOT NULL, "
	 "PRIMARY KEY (user_id, badge_id));"
	 "CREATE TABLE IF NOT EXISTS gb_boosts ("
	 "user_id TEXT PRIMARY KEY, mult REAL NOT NULL, expires INTEGER NOT NULL);",
	 nullptr, nullptr, nullptr);
}

static void usage() {
	std::cerr << "usage:\n"
	 << " axuctl badge grant <user_id> <badge_id>\n"
	 << " axuctl badge revoke <user_id> <badge_id>\n"
	 << " axuctl badge list <user_id>\n"
	 << " axuctl xpboost set <user_id> <mult> [hours] (0 or omitted = permanent)\n"
	 << " axuctl xpboost clear <user_id>\n"
	 << " axuctl xpboost info <user_id>\n"
	 << " axuctl bg set <guild_id> <user_id> <bg_name> <artist> <invert(0|1)>\n"
	 << " axuctl bg clear <guild_id> <user_id>\n"
	 << " axuctl bg info <guild_id> <user_id>\n";
}

static void exec_bind2(const char* sql, const char* a, const char* b) {
	sqlite3_stmt* s;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, a, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 2, b, -1, SQLITE_TRANSIENT);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}
}

static void setting_set(const char* guild_id, const char* key, const char* val) {
	const char* sql = "INSERT INTO guild_settings (guild_id, key, value) VALUES (?, ?, ?) "
	 "ON CONFLICT(guild_id, key) DO UPDATE SET value = ?;";
	sqlite3_stmt* s;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, guild_id, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 3, val, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 4, val, -1, SQLITE_TRANSIENT);
		sqlite3_step(s);
		sqlite3_finalize(s);
	}
}

static std::string setting_get(const char* guild_id, const char* key) {
	const char* sql = "SELECT value FROM guild_settings WHERE guild_id = ? AND key = ?;";
	sqlite3_stmt* s;
	std::string result;
	if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, guild_id, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
		if (sqlite3_step(s) == SQLITE_ROW) {
			const unsigned char* t = sqlite3_column_text(s, 0);
			if (t) result = reinterpret_cast<const char*>(t);
		}
		sqlite3_finalize(s);
	}
	return result;
}

int main(int argc, char* argv[]) {
	if (argc < 3) { usage(); return 1; }
	db_open();

	std::string domain = argv[1];
	std::string action = argv[2];

	if (domain == "badge") {
		if (argc < 4) { usage(); return 1; }
		std::string uid = argv[3];

		if (action == "list") {
			sqlite3_stmt* s;
			const char* sql = "SELECT badge_id FROM user_badges WHERE user_id = ? ORDER BY granted_at ASC;";
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				bool any = false;
				while (sqlite3_step(s) == SQLITE_ROW) {
					std::cout << reinterpret_cast<const char*>(sqlite3_column_text(s, 0)) << "\n";
					any = true;
				}
				sqlite3_finalize(s);
				if (!any) std::cout << "no badges for " << uid << "\n";
			}
		} else if (action == "grant") {
			if (argc < 5) { usage(); return 1; }
			const char* sql = "INSERT OR IGNORE INTO user_badges (user_id, badge_id, granted_at) VALUES (?, ?, ?);";
			sqlite3_stmt* s;
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(s, 2, argv[4], -1, SQLITE_TRANSIENT);
				sqlite3_bind_int64(s, 3, std::time(nullptr));
				sqlite3_step(s);
				sqlite3_finalize(s);
			}
			std::cout << "granted " << argv[4] << " to " << uid << "\n";
		} else if (action == "revoke") {
			if (argc < 5) { usage(); return 1; }
			exec_bind2("DELETE FROM user_badges WHERE user_id = ? AND badge_id = ?;", uid.c_str(), argv[4]);
			std::cout << "revoked " << argv[4] << " from " << uid << "\n";
		} else { usage(); return 1; }

	} else if (domain == "xpboost") {
		if (argc < 4) { usage(); return 1; }
		std::string uid = argv[3];

		if (action == "set") {
			if (argc < 5) { usage(); return 1; }
			double mult = std::stod(argv[4]);
			if (mult <= 1.0) { std::cerr << "mult must be > 1.0\n"; return 1; }
			int hours = (argc >= 6) ? std::stoi(argv[5]) : 0;
			long expires = (hours <= 0) ? 0 : std::time(nullptr) + hours * 3600;
			const char* sql = "INSERT INTO gb_boosts (user_id, mult, expires) VALUES (?, ?, ?) "
			 "ON CONFLICT(user_id) DO UPDATE SET mult = ?, expires = ?;";
			sqlite3_stmt* s;
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_double(s, 2, mult);
				sqlite3_bind_int64(s, 3, expires);
				sqlite3_bind_double(s, 4, mult);
				sqlite3_bind_int64(s, 5, expires);
				sqlite3_step(s);
				sqlite3_finalize(s);
			}
			if (expires == 0)
				std::cout << "set permanent " << mult << "x boost for " << uid << "\n";
			else
				std::cout << "set " << mult << "x boost for " << uid << " for " << hours << "h\n";
		} else if (action == "clear") {
			exec_bind2("DELETE FROM gb_boosts WHERE user_id = ?;", uid.c_str(), nullptr);
			std::cout << "cleared boost for " << uid << "\n";
		} else if (action == "info") {
			const char* sql = "SELECT mult, expires FROM gb_boosts WHERE user_id = ? AND (expires = 0 OR expires > ?);";
			sqlite3_stmt* s;
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_int64(s, 2, std::time(nullptr));
				if (sqlite3_step(s) == SQLITE_ROW) {
					double m = sqlite3_column_double(s, 0);
					long exp = sqlite3_column_int64(s, 1);
					std::cout << "active boost: " << m << "x";
					if (exp == 0) std::cout << " (permanent)";
					else std::cout << " expires " << exp;
					std::cout << "\n";
				} else {
					std::cout << "no active boost for " << uid << "\n";
				}
				sqlite3_finalize(s);
			}
		} else { usage(); return 1; }

	} else if (domain == "bg") {
		if (argc < 5) { usage(); return 1; }
		std::string gid = argv[3];
		std::string uid = argv[4];
		std::string bg_key = "bg_override_" + uid;
		std::string art_key = "bg_artist_" + uid;
		std::string inv_key = "bg_invert_" + uid;

		if (action == "set") {
			if (argc < 8) { usage(); return 1; }
			std::string invert = std::string(argv[7]) == "0" ? "false" : "true";
			setting_set(gid.c_str(), bg_key.c_str(), argv[5]);
			setting_set(gid.c_str(), art_key.c_str(), argv[6]);
			setting_set(gid.c_str(), inv_key.c_str(), invert.c_str());
			std::cout << "set bg for " << uid << " in guild " << gid << "\n"
			 << " bg: " << argv[5] << "\n"
			 << " artist: " << argv[6] << "\n"
			 << " invert: " << invert << "\n";
		} else if (action == "clear") {
			setting_set(gid.c_str(), bg_key.c_str(), "");
			setting_set(gid.c_str(), art_key.c_str(), "");
			setting_set(gid.c_str(), inv_key.c_str(), "false");
			std::cout << "cleared bg for " << uid << " in guild " << gid << "\n";
		} else if (action == "info") {
			std::string bg = setting_get(gid.c_str(), bg_key.c_str());
			std::string art = setting_get(gid.c_str(), art_key.c_str());
			std::string inv = setting_get(gid.c_str(), inv_key.c_str());
			if (bg.empty()) std::cout << "no custom bg for " << uid << " in guild " << gid << "\n";
			else std::cout << "bg: " << bg << "\n"
			 << "artist: " << art << "\n"
			 << "invert: " << inv << "\n";
		} else { usage(); return 1; }

	} else { usage(); return 1; }

	sqlite3_close(db);
	return 0;
}
