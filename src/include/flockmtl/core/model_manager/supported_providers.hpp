#pragma once
#include <string>
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
    if (provider == "openai" || provider == "default" || provider == "")
        return FLOCKMTL_OPENAI;
    if (provider == "azure")
        return FLOCKMTL_AZURE;
    if (provider == "ollama")
        return FLOCKMTL_OLLAMA;
    if (provider == "bedrock")
        return FLOCKMTL_AWS_BEDROCK;

    return FLOCKMTL_UNSUPPORTED_PROVIDER;
}
