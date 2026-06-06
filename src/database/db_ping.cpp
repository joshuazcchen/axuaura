// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include <sqlite3.h>

#include <string>
#include <vector>

#include "db.h"

namespace db {
	bool ping() {
		if (!db_ptr) return false;
		sqlite3_stmt* s;
		bool ok = false;
		if (sqlite3_prepare_v2(db_ptr, "SELECT 1;", -1, &s, nullptr) == SQLITE_OK) {
			ok = (sqlite3_step(s) == SQLITE_ROW);
			sqlite3_finalize(s);
		}
		return ok;
	}
} // namespace db
