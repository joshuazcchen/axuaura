// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <ctime>
#include <iostream>
#include <sqlite3.h>
#include <string>

#include "axuctl.h"

int cmd_xpboost(int argc, char* argv[]) {
	if (argc < 4) {
		usage();
		return 1;
	}

	std::string action = argv[2];
	std::string uid = argv[3];

	if (action == "set") {
		if (argc < 5) {
			usage();
			return 1;
		}
		double mult;
		try {
			mult = std::stod(argv[4]);
		} catch (...) {
			std::cerr << "invalid multiplier\n";
			return 1;
		}
		if (mult <= 1.0) {
			std::cerr << "mult must be > 1.0\n";
			return 1;
		}
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
			std::cout << "set " << mult << "x boost for " << uid << " for " << hours << "h (expires " << expires
					  << ")\n";
		return 0;

	} else if (action == "clear") {
		exec_sql("DELETE FROM gb_boosts WHERE user_id = ?;", uid.c_str());
		std::cout << "cleared boost for " << uid << "\n";
		return 0;

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
				if (exp == 0)
					std::cout << " (permanent)";
				else
					std::cout << " (expires " << exp << ")";
				std::cout << "\n";
			} else {
				std::cout << "no active boost for " << uid << "\n";
			}
			sqlite3_finalize(s);
		}
		return 0;
	}

	usage();
	return 1;
}
