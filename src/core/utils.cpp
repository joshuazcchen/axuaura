#include "utils.h"

namespace utils {
	bool is_admin(const dpp::slashcommand_t& event) {
		dpp::permission resolved = event.command.get_resolved_permission(event.command.get_issuing_user().id);
		return resolved.has(dpp::p_administrator);
	}
} // namespace utils
