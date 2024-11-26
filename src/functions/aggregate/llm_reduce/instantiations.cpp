#include "flockmtl/functions/aggregate/llm_reduce.hpp"

namespace flockmtl {

template void AggregateFunctionBase::Initialize<LlmReduce>(const duckdb::AggregateFunction& function,
                                                           duckdb::data_ptr_t state_p);
template void AggregateFunctionBase::Operation<LlmReduce>(duckdb::Vector[], duckdb::AggregateInputData&, idx_t,
                                                          duckdb::Vector&, idx_t);
template void AggregateFunctionBase::SimpleUpdate<LlmReduce>(duckdb::Vector[], duckdb::AggregateInputData&, idx_t,
                                                             duckdb::data_ptr_t, idx_t);
template void AggregateFunctionBase::Combine<LlmReduce>(duckdb::Vector&, duckdb::Vector&, duckdb::AggregateInputData&,
                                                        idx_t);

} // namespace flockmtl
