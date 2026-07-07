// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <iostream>
#include <sqlite3.h>
#include <string>

#include "axuctl.h"

int cmd_guild(int argc, char* argv[]) {
	if (argc < 4) {
		usage();
		return 1;
	}

	std::string action = argv[2];
	std::string gid = argv[3];

	if (action == "set") {
		if (argc < 6) {
			usage();
			return 1;
		}
		setting_set(gid.c_str(), argv[4], argv[5]);
		std::cout << "set " << argv[4] << " = " << argv[5] << " for guild " << gid << "\n";
		return 0;

	} else if (action == "get") {
		if (argc < 5) {
			usage();
			return 1;
		}
		std::string val = setting_get(gid.c_str(), argv[4]);
		if (val.empty())
			std::cout << argv[4] << " is not set for guild " << gid << "\n";
		else
			std::cout << argv[4] << " = " << val << "\n";
		return 0;

	} else if (action == "list") {
		const char* sql = "SELECT key, value FROM guild_settings WHERE guild_id = ? ORDER BY key ASC;";
		sqlite3_stmt* s;
		if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
			sqlite3_bind_text(s, 1, gid.c_str(), -1, SQLITE_TRANSIENT);
			bool any = false;
			while (sqlite3_step(s) == SQLITE_ROW) {
				std::cout << reinterpret_cast<const char*>(sqlite3_column_text(s, 0)) << " = "
						  << reinterpret_cast<const char*>(sqlite3_column_text(s, 1)) << "\n";
				any = true;
			}
			sqlite3_finalize(s);
			if (!any) std::cout << "no settings for guild " << gid << "\n";
		}
		return 0;
	}

	usage();
	return 1;
}
