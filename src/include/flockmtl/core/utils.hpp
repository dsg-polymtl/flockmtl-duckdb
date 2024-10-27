#pragma once
#include "nlohmann/json.hpp"

namespace flockmtl {
namespace core {
    bool get_provider_name_from_settings(const nlohmann::json& json, std::string& vendor_name);
}
}