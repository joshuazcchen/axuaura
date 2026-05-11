#pragma once
#include <dpp/dpp.h>

namespace voice {
void handle(const dpp::voice_state_update_t& event, dpp::cluster& bot);
}
