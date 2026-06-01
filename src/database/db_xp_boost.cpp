#include <ctime>
#include <sqlite3.h>

#include "db.h"

namespace db {

	void gb_set(dpp::snowflake user_id, double mult, long expires) {
		const char* sql = "INSERT INTO global_boosts (user_id, mult, expires) VALUES (?, ?, ?) "
						  "ON CONFLICT(user_id) DO UPDATE SET mult = ?, expires = ?;";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_double(s, 2, mult);
			sqlite3_bind_int64(s, 3, expires);
			sqlite3_bind_double(s, 4, mult);
			sqlite3_bind_int64(s, 5, expires);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	void gb_clear(dpp::snowflake user_id) {
		const char* sql = "DELETE FROM global_boosts WHERE user_id = ?;";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(s);
			sqlite3_finalize(s);
		}
	}

	double gb_get(dpp::snowflake user_id) {
		const char* sql = "SELECT mult FROM global_boosts WHERE user_id = ? AND (expires = 0 OR expires > ?);";
		sqlite3_stmt* s;
		double mult = 1.0;
		if (sqlite3_prepare_v2(db_ptr, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, std::to_string(user_id).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(s, 2, std::time(nullptr));
			if (sqlite3_step(s) == SQLITE_ROW) mult = sqlite3_column_double(s, 0);
			sqlite3_finalize(s);
		}
		return mult;
	}

} // namespace db
