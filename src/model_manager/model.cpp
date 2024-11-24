#include "flockmtl/model_manager/model.hpp"

namespace flockmtl {

Model::Model(const nlohmann::json &model_json) {
    LoadModelDetails(model_json);
    ConstructProvider();
}

void Model::LoadModelDetails(const nlohmann::json &model_json) {
    model_details_.provider_name = model_json.contains("provider") ? model_json.at("provider").dump() : "";
    model_details_.model_name = model_json.contains("model_name") ? model_json.at("model_name").dump() : "";
    model_details_.secret = LoadSecret(model_details_.provider_name);

    auto query_result = GetQueriedModel(model_details_.model_name, model_details_.provider_name);
    model_details_.model = std::get<0>(query_result);
    model_details_.context_window = std::get<1>(query_result);
    model_details_.max_output_tokens = std::get<2>(query_result);
    model_details_.temperature = 0.5;

    for (auto &[key, value] : model_json.items()) {
        if (key == "max_output_tokens") {
            model_details_.max_output_tokens = std::stoi(static_cast<std::string>(value));
        } else if (key == "temperature") {
            model_details_.temperature = std::stof(static_cast<std::string>(value));
        } else if (key == "provider" || key == "model_name") {
            continue;
        } else {
            throw std::invalid_argument("Invalid setting key: " + key);
        }
    }
}

std::string Model::LoadSecret(const std::string &provider_name) {
    auto query = "SELECT secret FROM "
                 "flockmtl_config.FLOCKMTL_SECRET_INTERNAL_TABLE "
                 "WHERE provider = '" +
                 provider_name + "'";

    auto query_result = core::CoreModule::GetConnection().Query(query);

    if (query_result->RowCount() == 0) {
        return "";
    }

    return query_result->GetValue(0, 0).ToString();
}

std::tuple<std::string, int32_t, int32_t> Model::GetQueriedModel(const std::string &model_name,
                                                                 const std::string &provider_name) {

    auto provider_name_lower = provider_name;
    std::transform(provider_name_lower.begin(), provider_name_lower.end(), provider_name_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (provider_name_lower == OLLAMA) {
        OllamaModel olam(false);
        if (!olam.validModel(model_name)) {
            throw std::runtime_error("Specified ollama model not deployed, please deploy before using it");
        }
        return {model_name, core::Config::default_context_window, core::Config::default_max_output_tokens};
    }

    std::string query = "SELECT model, model_args FROM "
                        "flockmtl_config.FLOCKMTL_MODEL_USER_DEFINED_INTERNAL_TABLE "
                        "WHERE model_name = '" +
                        model_name + "'";
    if (!provider_name.empty()) {
        query += " AND provider_name = '" + provider_name + "'";
    }

    auto &con = core::CoreModule::GetConnection();
    auto query_result = con.Query(query);

    if (query_result->RowCount() == 0) {
        query_result = con.Query("SELECT model, model_args FROM "
                                 "flockmtl_config.FLOCKMTL_MODEL_DEFAULT_INTERNAL_TABLE WHERE model_name = '" +
                                 model_name + "'");

        if (query_result->RowCount() == 0) {
            throw std::runtime_error("Model not found");
        }
    }

    auto model = query_result->GetValue(0, 0).ToString();
    auto model_args = nlohmann::json::parse(query_result->GetValue(1, 0).ToString());

    return {query_result->GetValue(0, 0).ToString(), model_args["context_window"], model_args["max_output_tokens"]};
}

void Model::ConstructProvider() {
    auto provider = GetProviderType(model_details_.provider_name);
    switch (provider) {
    case FLOCKMTL_OPENAI:
        provider_ = std::make_unique<OpenAIProvider>(model_details_);
        break;
    case FLOCKMTL_AZURE:
        provider_ = std::make_unique<AzureProvider>(model_details_);
        break;
    case FLOCKMTL_OLLAMA:
        provider_ = std::make_unique<OllamaProvider>(model_details_);
        break;
    default:
        throw std::invalid_argument("Unsupported provider: " + model_details_.provider_name);
    }
}

nlohmann::json Model::CallComplete(const std::string &prompt, bool json_response) {
    return provider_->CallComplete(prompt, json_response);
}

nlohmann::json Model::CallEmbedding(const std::vector<std::string> &inputs) { return provider_->CallEmbedding(inputs); }

} // namespace flockmtl
