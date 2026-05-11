#pragma once
#include <dpp/dpp.h>

namespace message {
void handle(const dpp::message_create_t& event, dpp::cluster& bot);
}
