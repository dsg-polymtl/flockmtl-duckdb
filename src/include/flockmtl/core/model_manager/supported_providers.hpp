#pragma once
#include <string>

const std::string OLLAMA = "ollama";
const std::string OPENAI = "openai";
const std::string AZURE = "azure";
const std::string DEFAULT_PROVIDER = "default";
const std::string EMPTY_PROVIDER = "";
const std::string BEDROCK = "bedrock";

enum SupportedProviders {
    FLOCKMTL_OPENAI = 0,
    FLOCKMTL_AZURE,
    FLOCKMTL_AWS_BEDROCK,
    FLOCKMTL_OLLAMA,
    FLOCKMTL_UNSUPPORTED_PROVIDER,
    FLOCKMTL_SUPPORTED_PROVIDER_COUNT
};

inline SupportedProviders GetProviderType(std::string provider) {
    std::transform(provider.begin(), provider.end(), provider.begin(), [](unsigned char c) { return std::tolower(c); });
    if (provider == OPENAI || provider == DEFAULT_PROVIDER || provider == EMPTY_PROVIDER)
        return FLOCKMTL_OPENAI;
    if (provider == AZURE)
        return FLOCKMTL_AZURE;
    if (provider == OLLAMA)
        return FLOCKMTL_OLLAMA;
    if (provider == BEDROCK)
        return FLOCKMTL_AWS_BEDROCK;

    return FLOCKMTL_UNSUPPORTED_PROVIDER;
}
