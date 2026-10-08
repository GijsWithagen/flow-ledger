#pragma once

#include "result.hpp"
#include <cstdint>
#include <string_view>
#include <unordered_map>

enum TransactionStatus
{
    Unknown,
    Pending,
    Completed,
    Failed,
};

struct Transaction
{
    uint64_t transaction_id;
    uint64_t source_account;
    uint64_t destination_account;
    int64_t amount;
    uint64_t timestamp;
    TransactionStatus status;
};

class TransactionEngine
{
private:
    std::unordered_map<uint64_t, int64_t> account_balances_;
    std::unordered_map<uint64_t, Transaction> transactions_;
    std::unordered_map<std::string_view, uint64_t> idempotency_key_to_transaction_id_;

public:
    void create_account(
        uint64_t account_id,
        int64_t initial_balance);

    Result transfer(
        uint64_t transaction_id,
        uint64_t source_account,
        uint64_t destination_account,
        int64_t amount,
        std::string_view idempotency_key);

    TransactionStatus status(
        uint64_t transaction_id) const;

    int64_t balance(
        uint64_t account_id) const;

    Transaction get_transaction(
        uint64_t transaction_id) const;
};