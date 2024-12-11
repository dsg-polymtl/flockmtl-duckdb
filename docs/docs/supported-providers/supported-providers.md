# Supported Providers

FlockMTL supports the following providers for semantic analytics tasks such as text generation, embedding creation, and ranking:

- [**Ollama**](/docs/supported-providers/ollama)
- [**OpenAI & OpenAI Compatible Providers**](/docs/supported-providers/openai)
- [**Azure**](/docs/supported-providers/azure)

## Manage Provider's Secrets

FlockMTL securely stores and manages sensitive information like API keys and credentials using the [secret manager](/docs/resource-management/secret-management). This enables secure integration with multiple service providers.

## Provider Interaction with Resources

Each model in FlockMTL is linked to a specific provider. Secrets are used to authenticate API requests, ensuring reliable operation and access to provider-specific features.
