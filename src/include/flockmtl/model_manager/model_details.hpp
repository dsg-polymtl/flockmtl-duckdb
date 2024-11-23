#pragma once

#include <string>

namespace flockmtl {

struct ModelDetails {
    std::string provider_name;
    std::string model_name;
    std::string model;
    int32_t context_window;
    int32_t max_output_tokens;
    float temperature;
    std::string secret;
};

} // namespace flockmtl