// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <sqlite3.h>

#include "db.h"

namespace db {
	extern sqlite3* db_ptr;

	void optimize() {
		if (db_ptr) { sqlite3_exec(db_ptr, "PRAGMA optimize;", nullptr, nullptr, nullptr); }
	}
} // namespace db
