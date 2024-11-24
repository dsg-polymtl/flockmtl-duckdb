#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <utility>
#include <nlohmann/json.hpp>

#include "flockmtl/core/module.hpp"
#include "flockmtl/core/config/config.hpp"
#include "flockmtl/model_manager/repository.hpp"
#include "flockmtl/model_manager/providers/adapters/openai.hpp"
#include "flockmtl/model_manager/providers/adapters/azure.hpp"
#include "flockmtl/model_manager/providers/adapters/ollama.hpp"
#include "flockmtl/model_manager/providers/handlers/ollama.hpp"
#include "duckdb/main/connection.hpp"

namespace flockmtl {

class Model {
public:
    std::shared_ptr<IProvider> provider_;
    ModelDetails model_details_;

    explicit Model(const nlohmann::json &model_json);
    nlohmann::json CallComplete(const std::string &prompt, const bool json_response = true);
    nlohmann::json CallEmbedding(const std::vector<string> &inputs);

private:
    void ConstructProvider();
    void LoadModelDetails(const nlohmann::json &model_json);
    std::string LoadSecret(const std::string &provider);
    std::tuple<std::string, std::string, int32_t, int32_t> GetQueriedModel(const std::string &model_name);
};

} // namespace flockmtl
