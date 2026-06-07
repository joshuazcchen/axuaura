// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "backup.h"

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>

namespace backup {

	static std::string db_path() {
		const char* p = std::getenv("DATABASE_PATH");
		return p ? p : "database.sqlite";
	}

	static int backup_days() {
		const char* v = std::getenv("BACKUP_DAYS");
		if (!v) return 3;
		try {
			int n = std::stoi(v);
			return (n > 0) ? n : 3;
		} catch (...) { return 3; }
	}

	static std::string today_stamp() {
		std::time_t t = std::time(nullptr);
		char buf[16];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
		return buf;
	}

	void do_backup() {
		namespace fs = std::filesystem;

		std::string src = db_path();
		if (!fs::exists(src)) {
			std::cerr << "source db not found: " << src << std::endl;
			return;
		}

		fs::path sp(src);
		fs::path dir = sp.parent_path();
		std::string stem = sp.stem().string();
		std::string ext = sp.extension().string();
		fs::path dst = dir / (stem + "_" + today_stamp() + ext);

		if (fs::exists(dst)) return;

		try {
			fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
			std::cout << "backup wrote: " << dst << std::endl;
		} catch (const std::exception& e) {
			std::cerr << "backup failed: " << e.what() << std::endl;
			return;
		}

		int keep = backup_days();
		std::regex pat(stem + "_(\\d{4}-\\d{2}-\\d{2})" + ext + "$");
		std::vector<std::pair<std::string, fs::path>> found;
		try {
			for (auto& entry : fs::directory_iterator(dir.empty() ? "." : dir)) {
				std::string fn = entry.path().filename().string();
				std::smatch m;
				if (std::regex_match(fn, m, pat)) found.emplace_back(m[1].str(), entry.path());
			}
		} catch (...) {}

		std::sort(found.begin(), found.end());
		while ((int)found.size() > keep) {
			try {
				fs::remove(found.front().second);
			} catch (...) {}
			found.erase(found.begin());
		}
	}

} // namespace backup
