#include "flockmtl/core/utils.hpp"
#include <string>
namespace flockmtl {
namespace core {

const std::string provider_key = "provider";

static bool helper (const nlohmann::json& json, std::string& provider_name){
    if (json.contains(provider_key)) {
        provider_name = json.at(provider_key).get<std::string>();
        return true;
    }
    return false;
}

bool get_provider_name_from_settings(const nlohmann::json& json, std::string& provider_name) {
    return helper (json, provider_name);
}

} // namespace core
} // namespace flockmtl
