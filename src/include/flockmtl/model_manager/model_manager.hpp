#pragma once

#include <nlohmann/json.hpp>
#include <utility>
#include <tuple>
#include <vector>
#include <string>

#include "flockmtl/model_manager/providers/provider.hpp"
#include "flockmtl/core/module.hpp"
#include "flockmtl/core/config/config.hpp"
#include "flockmtl/model_manager/model_details.hpp"
#include "flockmtl/model_manager/supported/providers.hpp"
#include "flockmtl/model_manager/providers/handlers/ollama.hpp"

namespace flockmtl {

class ModelManager {
public:
    std::unique_ptr<IProvider> provider_;
    ModelDetails model_details_;

    explicit ModelManager(const nlohmann::json &model_json);
    nlohmann::json CallComplete(const std::string &prompt, const bool json_response = true);
    nlohmann::json CallEmbedding(const std::vector<string> &inputs);

private:
    void ConstructProvider();
    void LoadModelDetails(const nlohmann::json &model_json);
    std::string LoadSecret(const std::string &provider);
    std::tuple<std::string, int32_t, int32_t> GetQueriedModel(const std::string &model_name,
                                                              const std::string &provider_name);
};

} // namespace flockmtl
