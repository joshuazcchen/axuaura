// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>
#include <string>

#include "axuctl.h"

int cmd_bg(int argc, char* argv[]) {
	if (argc < 4) {
		usage();
		return 1;
	}

	std::string scope = argv[2];
	std::string action = argv[3];

	if (scope == "global") {
		if (argc < 5) {
			usage();
			return 1;
		}
		std::string uid = argv[4];

		if (action == "set") {
			if (argc < 8) {
				usage();
				return 1;
			}
			int iv = (std::string(argv[7]) == "0") ? 0 : 1;
			const char* sql = "INSERT INTO global_banners (user_id, filename, artist, invert) VALUES (?, ?, ?, ?)"
							  " ON CONFLICT(user_id) DO UPDATE SET filename=?, artist=?, invert=?";
			sqlite3_stmt* s;
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(s, 2, argv[5], -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(s, 3, argv[6], -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(s, 4, iv);
				sqlite3_bind_text(s, 5, argv[5], -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(s, 6, argv[6], -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(s, 7, iv);
				sqlite3_step(s);
				sqlite3_finalize(s);
			}
			std::cout << "set global bg for " << uid << "\nbg: " << argv[5] << "\nartist: " << argv[6]
					  << "\ninvert: " << iv << "\n";
			return 0;

		} else if (action == "clear") {
			const char* sql = "DELETE FROM global_banners WHERE user_id = ?";
			sqlite3_stmt* s;
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_step(s);
				sqlite3_finalize(s);
			}
			std::cout << "cleared global bg for " << uid << "\n";
			return 0;

		} else if (action == "info") {
			const char* sql = "SELECT filename, artist, invert FROM global_banners WHERE user_id = ?";
			sqlite3_stmt* s;
			bool found = false;
			if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(s, 1, uid.c_str(), -1, SQLITE_TRANSIENT);
				if (sqlite3_step(s) == SQLITE_ROW) {
					found = true;
					std::cout << "bg: "
							  << (sqlite3_column_text(s, 0) ? reinterpret_cast<const char*>(sqlite3_column_text(s, 0))
															: "")
							  << "\nartist: "
							  << (sqlite3_column_text(s, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(s, 1))
															: "")
							  << "\ninvert: " << sqlite3_column_int(s, 2) << "\n";
				}
				sqlite3_finalize(s);
			}
			if (!found) std::cout << "no global bg for " << uid << "\n";
			return 0;
		}

	} else if (scope == "server") {
		if (argc < 6) {
			usage();
			return 1;
		}
		std::string gid = argv[4];
		std::string uid = argv[5];
		std::string bg_key = "bg_override_" + uid;
		std::string art_key = "bg_artist_" + uid;
		std::string inv_key = "bg_invert_" + uid;

		if (action == "set") {
			if (argc < 9) {
				usage();
				return 1;
			}
			std::string invert = (std::string(argv[8]) == "0") ? "false" : "true";
			setting_set(gid.c_str(), bg_key.c_str(), argv[6]);
			setting_set(gid.c_str(), art_key.c_str(), argv[7]);
			setting_set(gid.c_str(), inv_key.c_str(), invert.c_str());
			std::cout << "set server bg for " << uid << " in guild " << gid << "\n"
					  << "bg: " << argv[6] << "\nartist: " << argv[7] << "\ninvert: " << invert << "\n";
			return 0;

		} else if (action == "clear") {
			setting_set(gid.c_str(), bg_key.c_str(), "");
			setting_set(gid.c_str(), art_key.c_str(), "");
			setting_set(gid.c_str(), inv_key.c_str(), "false");
			std::cout << "cleared server bg for " << uid << " in guild " << gid << "\n";
			return 0;

		} else if (action == "info") {
			std::string bg = setting_get(gid.c_str(), bg_key.c_str());
			std::string art = setting_get(gid.c_str(), art_key.c_str());
			std::string inv = setting_get(gid.c_str(), inv_key.c_str());
			if (bg.empty())
				std::cout << "no server bg for " << uid << " in guild " << gid << "\n";
			else
				std::cout << "bg: " << bg << "\nartist: " << art << "\ninvert: " << inv << "\n";
			return 0;
		}
	}

	usage();
	return 1;
}
