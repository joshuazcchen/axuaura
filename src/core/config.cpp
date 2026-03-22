#include "config.h"

// WHY IS THIS HERE??????? WHY DID I PUT THIS HERE???????? TODO TODO TODO TODO TODO TODO TODO TODO

namespace config {
    std::vector<dpp::snowflake> SPECIALS = { 
        802736184546689044 
    };

    std::vector<dpp::snowflake> ALLOWED_CHANNELS = { 
        1482901206018822296, 1469591365645111478, 1480102566871433317, 1480688308811071732, 1482523765308002314, 1482653036861067396,
        1482634709526904975, 1482470065415786628, 1469597924596912188, 1482470983813300465, 1469593150858465394, 1170145267224428635
    };

    dpp::snowflake NON_ENG_CH = 1482901206018822296; 
    dpp::snowflake LOG_CH = 1485390883841904740; 

    const std::vector<std::string> RELIABLE_PROVIDERS = {
        "tenor.com", "giphy.com", "youtube.com", "youtu.be", "klipy.com", "twitter.com", "x.com", "instagram.com", "tiktok.com"
    };

    const std::vector<std::string> AURA_LOSSES = {
        "**HOLY AURA LOSS <:heartem:1485404606480515172>**", "aura loss <:heartem:1485404606480515172>", "<:heartem:1485404606480515172> <:skem:1485404723501863085> aura loss <:downvote:1485404530924589228>", "Aura loss <:heartem:1485404606480515172>", "damn you got less aura than axuaxi after that <:heartem:1485404606480515172>", "holy aura loss <:heartem:1485404606480515172>"
    };

    const std::vector<std::string> SPANISH_LOSS = {
        "adios aura <:heartem:1485404606480515172>", "au rev'aura <:heartem:1485404606480515172>", "AURA ⬇️  ", "你的aura减少。", "Aura berkurang <:heartem:1485404606480515172>"
    };
}
