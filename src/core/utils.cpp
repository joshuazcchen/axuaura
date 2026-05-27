#include "utils.h"

namespace utils {
	bool is_admin(const dpp::slashcommand_t& event) {
		dpp::permission resolved = event.command.get_resolved_permission(event.command.get_issuing_user().id);
		return resolved.has(dpp::p_administrator);
	}

	static std::string::size_type v_st(const std::string& j, const std::string& key) {
		std::string search = "\"" + key + "\"";
		auto kpos = j.find(search);
		if (kpos == std::string::npos) return std::string::npos;
		auto col = j.find(':', kpos + search.size());
		if (col == std::string::npos) return std::string::npos;
		auto vpos = j.find_first_not_of(" \n\r\t", col + 1);
		return vpos;
	}

	std::string json_str(const std::string& j, const std::string& key) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos || j[vpos] != '"') return "";
		auto q2 = j.find('"', vpos + 1);
		if (q2 == std::string::npos) return "";
		return j.substr(vpos + 1, q2 - vpos - 1);
	}

	bool json_bool(const std::string& j, const std::string& key, bool df) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos) return df;
		if (j.compare(vpos, 4, "true") == 0) return true;
		return false;
	}

	int json_int(const std::string& j, const std::string& key, int df) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos) return df;
		try {
			return std::stoi(j.substr(vpos));
		} catch (...) { return df; }
	}

	double json_doub(const std::string& j, const std::string& key, double df) {
		auto vpos = v_st(j, key);
		if (vpos == std::string::npos) return df;
		try {
			return std::stod(j.substr(vpos));
		} catch (...) { return df; }
	}
} // namespace utils
