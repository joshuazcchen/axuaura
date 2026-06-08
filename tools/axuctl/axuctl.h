// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

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
