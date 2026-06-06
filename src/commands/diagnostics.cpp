// Copyright (c) 2026 Joshua Chen. All rights reserved.
// Licensed under the PolyForm Noncommercial License 1.0.0.

#include "commands.h"

#include "db.h"
#include "image.h"

#include <chrono>
#include <ctime>
#include <dpp/dpp.h>
#include <fstream>
#include <malloc.h>
#include <sstream>
#include <string>

namespace commands {
	static std::chrono::steady_clock::time_point boot_time;

	void diagnostics_set_boot() { boot_time = std::chrono::steady_clock::now(); }

	static std::string row(const std::string& label, const std::string& val, int width = 16) {
		std::string padded = label;
		padded.resize(width, ' ');
		return "  " + padded + val + "\n";
	}

	static int64_t img_latency_ms() {
		auto t0 = std::chrono::high_resolution_clock::now();
		{ image::img_gen_card("", "diagnostics", 1, 0, 100, 0.0f); }

		malloc_trim(0);
		auto t1 = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
	}

	static std::string uptime_str(int64_t seconds) {
		int d = seconds / 86400;
		int h = (seconds % 86400) / 3600;
		int m = (seconds % 3600) / 60;
		int s = seconds % 60;
		std::string out;
		if (d) out += std::to_string(d) + "d ";
		if (h) out += std::to_string(h) + "h ";
		if (m) out += std::to_string(m) + "m ";
		out += std::to_string(s) + "s";
		return out;
	}

	static long rss_kb() {
		std::ifstream f("/proc/self/status");
		if (!f.is_open()) return -1;
		std::string line;
		while (std::getline(f, line)) {
			if (line.rfind("VmRSS:", 0) == 0) {
				std::istringstream ss(line);
				std::string label;
				long val;
				ss >> label >> val;
				return val;
			}
		}
		return -1;
	}

	static std::string cpu_temp() {
		FILE* p = popen("sensors -j 2>/dev/null", "r");
		if (!p) return "unavailable";
		std::string out;
		char buf[256];
		while (fgets(buf, sizeof(buf), p))
			out += buf;
		pclose(p);
		auto pos = out.find("CPUTIN");
		if (pos == std::string::npos) return "unavailable";
		auto inp = out.find("temp1_input", pos);
		if (inp == std::string::npos) return "unavailable";
		auto colon = out.find(':', inp);
		if (colon == std::string::npos) return "unavailable";
		double val = std::stod(out.substr(colon + 1));
		std::ostringstream ss;
		ss << std::fixed;
		ss.precision(1);
		ss << val << "°C";
		return ss.str();
	}

	static std::string gpu_temp() {
		FILE* p = popen("nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader 2>/dev/null", "r");
		if (!p) return "unavailable";
		char buf[64] = {};
		fgets(buf, sizeof(buf), p);
		pclose(p);
		std::string val(buf);
		val.erase(val.find_last_not_of(" \n\r") + 1);
		return val.empty() ? "unavailable" : val + ".0°C";
	}

	static std::string fmt_mem(long kb) {
		if (kb < 0) return "unavailable";
		if (kb < 1024) return std::to_string(kb) + " KB";
		return std::to_string(kb / 1024) + " MiB (" + std::to_string(kb) + " KiB)";
	}

	static std::string fmt_db(int64_t us) {
		if (us < 0) return "failed";
		if (us < 1000) return std::to_string(us) + "μs";
		return std::to_string(us / 1000) + "." + std::to_string((us % 1000) / 10) + "ms";
	}

	dpp::slashcommand diagnostics_def(dpp::cluster& bot) {
		return dpp::slashcommand("diagnostics", "bot health stats", bot.me.id);
	}

	void handle_diagnostics(const dpp::slashcommand_t& event, dpp::cluster& bot) {
		event.thinking(false);

		long mem = rss_kb();

		int64_t db_us = -1;
		{
			auto t0 = std::chrono::high_resolution_clock::now();
			bool ok = db::ping();
			auto t1 = std::chrono::high_resolution_clock::now();
			if (ok) db_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
		}

		uint64_t ws_ms = (uint64_t)(bot.rest_ping * 1000.0);
		uint64_t img_ms = img_latency_ms();
		auto rest_t0 = std::chrono::steady_clock::now();

		bot.current_user_get([event, ws_ms, db_us, rest_t0, img_ms, mem](const dpp::confirmation_callback_t&) {
			auto rest_t1 = std::chrono::steady_clock::now();
			int64_t rest_ms = std::chrono::duration_cast<std::chrono::milliseconds>(rest_t1 - rest_t0).count();

			int64_t up_secs =
				std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - boot_time).count();

			std::string cpu_t = cpu_temp();
			std::string gpu_t = gpu_temp();

			size_t guild_count = dpp::get_guild_count();
			size_t user_count = dpp::get_user_count();
			size_t role_count = dpp::get_role_count();

			std::string build_ts = std::string(__DATE__) + " " + std::string(__TIME__);

			std::string db_dot = db_us >= 0 ? "🟢" : "🔴";
			std::string rest_dot = rest_ms < 300 ? "🟢" : "🟡";
			std::string img_dot = img_ms < 300 ? "🟢" : "🔴";

			std::string content;
			content += "## diagnostics\n";
			content += rest_dot + " **rest** " + std::to_string(rest_ms) + "ms" + "  ●  " + db_dot + " **db** " +
					   fmt_db(db_us) + "  ●  " + img_dot + " **img gen** " + std::to_string(img_ms) + "ms\n";
			content += "\n";
			content += "**uptime** " + uptime_str(up_secs) + "  ●  **memory** " + fmt_mem(mem) + "\n";
			content += "\n";
			content += "**guilds** " + std::to_string(guild_count) + "  ●  **roles** " + std::to_string(role_count) +
					   "  ●  **users** " + std::to_string(user_count) + "\n";
			content += "**cpu** " + cpu_t + "  ●  **gpu** " + gpu_t + "\n";
			content += "\n";
			content += "-# **build " + build_ts + "**";

			event.edit_original_response(dpp::message(content));
		});
	}

} // namespace commands
