#pragma once
#include "nlohmann/json.hpp"

namespace flockmtl {
namespace core {
    bool is_provider_settings_available(const nlohmann::json& json);
}
}