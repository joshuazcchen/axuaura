// Copyright (c) 2026 Joshua Chen.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once
#include <sqlite3.h>
#include <string>

extern sqlite3* db;

void db_open();
void usage();
int cmd_badge(int argc, char* argv[]);
int cmd_xpboost(int argc, char* argv[]);
int cmd_bg(int argc, char* argv[]);
int cmd_guild(int argc, char* argv[]);
void exec_sql(const char* sql, const char* a, const char* b = nullptr);
void setting_set(const char* guild_id, const char* key, const char* val);
std::string setting_get(const char* guild_id, const char* key);
