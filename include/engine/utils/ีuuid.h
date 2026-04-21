#ifndef C8351F68_6FBD_48F1_BCE2_3296EB503F25
#define C8351F68_6FBD_48F1_BCE2_3296EB503F25

#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

inline std::string generate_uuid()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << dis(gen) << "-" << std::setw(4)
       << (dis(gen) & 0xFFFF) << "-" << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << "-"
       << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << "-" << std::setw(12)
       << ((uint64_t)dis(gen) << 32 | dis(gen));
    return ss.str();
}

#endif /* C8351F68_6FBD_48F1_BCE2_3296EB503F25 */
