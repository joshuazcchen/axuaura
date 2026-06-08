// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <ctime>
#include <iostream>
#include <sqlite3.h>
#include <string>

#include "axuctl.h"

int cmd_badge(int argc, char* argv[]) {
	if (argc < 4) {
		usage();
		return 1;
	}

	std::string action = argv[2];
	std::string uid = argv[3];

	if (action == "list") {
		const char* sql = "SELECT badge_id, granted_at FROM user_badges WHERE user_id = ? ORDER BY granted_at ASC;";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
			bool any = false;
			while (sqlite3_step(s) == SQLITE_ROW) {
				std::cout << reinterpret_cast<const char*>(sqlite3_column_text(s, 0)) << "(granted "
						  << sqlite3_column_int64(s, 1) << ")\n";
				any = true;
			}
			sqlite3_finalize(s);
			if (!any) std::cout << "no badges for " << uid << "\n";
		}
		return 0;

	} else if (action == "grant") {
		if (argc < 5) {
			usage();
			return 1;
		}
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
		return 0;

	} else if (action == "revoke") {
		if (argc < 5) {
			usage();
			return 1;
		}
		exec_sql("DELETE FROM user_badges WHERE user_id = ? AND badge_id = ?;", uid.c_str(), argv[4]);
		std::cout << "revoked " << argv[4] << " from " << uid << "\n";
		return 0;
	}

	usage();
	return 1;
}
