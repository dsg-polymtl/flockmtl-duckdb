#include "flockmtl/secret_manager/secret_manager.hpp"
#include <unordered_map>
#include "flockmtl/core/config.hpp"

#include <duckdb/main/secret/secret_manager.hpp>

namespace flockmtl {

const SecretDetails openai_secret_details = {"openai", "flockmtl", "openai://", {"api_key"}, {"api_key"}, {"api_key"}};

const SecretDetails azure_secret_details = {"azure_llm",    "flockmtl",
                                            "azure_llm://", {"api_key", "resource_name", "api_version"},
                                            {"api_key"},    {"api_key", "resource_name", "api_version"}};

const SecretDetails ollama_secret_details = {"ollama", "flockmtl", "ollama://", {"api_url"}, {"api_url"}, {"api_url"}};

const std::vector<const SecretDetails*> secret_details_list = {&openai_secret_details, &azure_secret_details,
                                                               &ollama_secret_details};

SecretManager::SupportedProviders SecretManager::GetProviderType(std::string provider) {
    if (provider == "openai") {
        return OPENAI;
    } else if (provider == "azure_llm") {
        return AZURE;
    } else if (provider == "ollama") {
        return OLLAMA;
    } else {
        throw duckdb::InvalidInputException("Unsupported secret type: %s", provider.c_str());
    }
}

void SecretManager::Register(duckdb::DatabaseInstance& instance) {
    RegisterSecretType(instance);
    RegisterSecretFunction(instance);
}

void SecretManager::RegisterSecretType(duckdb::DatabaseInstance& instance) {
    for (const auto& secret_details : secret_details_list) {
        duckdb::SecretType secret_type;
        secret_type.name = secret_details->type;
        secret_type.deserializer = duckdb::KeyValueSecret::Deserialize<duckdb::KeyValueSecret>;
        secret_type.default_provider = secret_details->provider;

        duckdb::ExtensionUtil::RegisterSecretType(instance, secret_type);
    }
}

void SecretManager::RegisterSecretFunction(duckdb::DatabaseInstance& instance) {
    for (const auto& secret_details : secret_details_list) {
        duckdb::CreateSecretFunction secret_function = {secret_details->type, secret_details->provider, CreateSecret};

        for (const auto& field : secret_details->fields) {
            secret_function.named_parameters[field] = duckdb::LogicalType::VARCHAR;
        }

        duckdb::ExtensionUtil::RegisterFunction(instance, secret_function);
    }
}

duckdb::unique_ptr<duckdb::BaseSecret> SecretManager::CreateSecret(duckdb::ClientContext& context,
                                                                   duckdb::CreateSecretInput& input) {
    const SecretDetails* selected_details = nullptr;

    if (input.type == openai_secret_details.type) {
        selected_details = &openai_secret_details;
    } else if (input.type == azure_secret_details.type) {
        selected_details = &azure_secret_details;
    } else if (input.type == ollama_secret_details.type) {
        selected_details = &ollama_secret_details;
    } else {
        throw duckdb::InvalidInputException("Unsupported secret type: %s", input.type.c_str());
    }

    ValidateRequiredFields(input, selected_details->required_fields);

    auto prefix_paths = input.scope;
    if (prefix_paths.empty()) {
        prefix_paths.push_back(selected_details->scope);
    }

    auto secret = ConstructBaseSecret(prefix_paths, *selected_details, input.type, input.provider, input.name);

    for (const auto& field : selected_details->fields) {
        auto lookup = input.options.find(field);
        if (lookup != input.options.end()) {
            secret->secret_map[field] = lookup->second;
        }
    }

    return std::move(secret);
}

duckdb::unique_ptr<duckdb::KeyValueSecret>
SecretManager::ConstructBaseSecret(duckdb::vector<std::string>& prefix_paths_p, const SecretDetails& details,
                                   std::string& type, std::string& provider, std::string& name) {
    auto secret = duckdb::make_uniq<duckdb::KeyValueSecret>(prefix_paths_p, type, provider, name);
    duckdb::case_insensitive_set_t redact_fields_set(details.redact_fields.begin(), details.redact_fields.end());
    secret->redact_keys = std::move(redact_fields_set);
    return secret;
}

std::unordered_map<std::string, std::string> SecretManager::GetSecret(std::string secret_name) {
    std::unordered_map<std::string, std::string> secret_map;
    auto& instance = Config::db;
    auto& secret_manager = instance->GetSecretManager();
    auto transaction = duckdb::CatalogTransaction::GetSystemTransaction(*instance);
    auto secret = secret_manager.GetSecretByName(transaction, secret_name);
    if (secret == nullptr) {
        throw duckdb::InvalidInputException("Secret not found : %s", secret_name.c_str());
    }
    const auto kv_secret = dynamic_cast<const duckdb::KeyValueSecret&>(*secret->secret);

    auto provider = secret->secret->GetType();
    auto providerType = SecretManager::GetProviderType(provider);
    SecretDetails secret_details;
    switch (providerType) {
    case SupportedProviders::OPENAI:
        secret_details = openai_secret_details;
        break;
    case SupportedProviders::AZURE:
        secret_details = azure_secret_details;
        break;
    case SupportedProviders::OLLAMA:
        secret_details = ollama_secret_details;
        break;
    default:
        throw duckdb::InvalidInputException("Unsupported secret type: %s", provider.c_str());
    }

    for (auto field : secret_details.fields) {
        secret_map[field] = kv_secret.TryGetValue(field).ToString();
    }

    return secret_map;
}

void SecretManager::ValidateRequiredFields(const duckdb::CreateSecretInput& input,
                                           const std::vector<std::string>& required_fields) {
    for (const auto& field : required_fields) {
        auto lookup = input.options.find(field);
        if (lookup == input.options.end() || lookup->second.ToString().empty()) {
            throw duckdb::InvalidInputException("Missing required field: '%s'", field.c_str());
        }
    }
}

} // namespace flockmtl
