#pragma once
#include <dpp/dpp.h>

#include <string>

namespace utils {
	bool is_admin(const dpp::slashcommand_t& event);
}
