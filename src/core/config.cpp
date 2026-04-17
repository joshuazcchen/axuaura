#include "config.h"

// WHY IS THIS HERE??????? WHY DID I PUT THIS HERE???????? TODO TODO TODO TODO TODO TODO TODO TODO

namespace config {
    std::vector<dpp::snowflake> SPECIALS = { 
        802736184546689045ULL // this is not really necessary since this is just neat's id + 1, technically I should fix this but im too lazy to. 
    };

    // TODO: find the solution to not need to wrap everything with an unsignedlonglong tag because for some stupid reason Alpine thinks differently and it runs on deb13 but not alpine??? musl is gonna give me an aneurysm.
    std::vector<dpp::snowflake> ALLOWED_CHANNELS = { 
        1482901206018822296ULL, 1469591365645111478ULL, 1480102566871433317ULL, 1480688308811071732ULL, 1482523765308002314ULL, 1482653036861067396ULL,
        1482634709526904975ULL, 1482470065415786628ULL, 1469597924596912188ULL, 1482470983813300465ULL, 1469593150858465394ULL, 1170145267224428635ULL
    };

    dpp::snowflake NON_ENG_CH = 1482901206018822296ULL; 
    dpp::snowflake LOG_CH = 1485390883841904740ULL; 

    dpp::snowflake LEADER = 1489380735482728688ULL;
    dpp::snowflake NUM2 = 1489380736363532408ULL;
    dpp::snowflake NUM3 = 1489380734467838073ULL;
    dpp::snowflake LOSER = 1489380737361776640ULL;
    dpp::snowflake BOT2 = 1489380733137977424ULL;
    dpp::snowflake BOT3 = 1489380730013220974ULL;

    std::vector<dpp::snowflake> STUPID_ROLES = {
        LEADER, NUM2, NUM3, LOSER, BOT2, BOT3
    };

    int XP_COOLDOWN = 60;
    int XP_MIN = 15;
    int XP_MAX = 30;

    const std::map<int, dpp::snowflake> LVL_ROLES = {
        {5, 1482905611535519877ULL},
        {10, 1482903838443835452ULL},
        {15, 1482903879686426876ULL},
    };

    dpp::snowflake LVL_CH = 1482858189035667628ULL;

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
