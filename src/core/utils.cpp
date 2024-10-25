#include "flockmtl/core/utils.hpp"
#include <string>
namespace flockmtl {
namespace core {

const std::string provider_key = "provider";
bool is_provider_settings_available (const nlohmann::json& json){
    return json.size() == 1 and json.begin().key() == provider_key and !json[provider_key].empty();
}

} // namespace core
} // namespace flockmtl
