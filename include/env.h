#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace env {
	void load(const std::string& path = ".env");

	std::string env_str(const std::string& key, const std::string& fallback = "");
	int env_int(const std::string& key, int fallback = 0);
	uint64_t env_sf(const std::string& key, uint64_t fallback = 0);
	std::vector<uint64_t> env_sf_vec(const std::string& key);
} // namespace env
