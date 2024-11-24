#pragma once

#include <nlohmann/json.hpp>
#include <flockmtl/model_manager/repository.hpp>

namespace flockmtl {

class IProvider {
public:
    ModelDetails model_details_;

    explicit IProvider(const ModelDetails &model_details) : model_details_(model_details) {};
    virtual ~IProvider() = default;

    virtual nlohmann::json CallComplete(const std::string &prompt, bool json_response) = 0;
    virtual nlohmann::json CallEmbedding(const std::vector<std::string> &inputs) = 0;
};

class ExceededMaxOutputTokensError : public std::exception {
public:
    const char *what() const noexcept override {
        return "The response exceeded the max_output_tokens length; increase your max_output_tokens parameter.";
    }
};

} // namespace flockmtl
