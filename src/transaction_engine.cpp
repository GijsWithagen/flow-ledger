#include "flow-ledger/transaction_engine.hpp"
#include <chrono>
#include <iostream>

inline uint64_t nanosecondNow()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    return static_cast<uint64_t>(nanoseconds.count());
}

void TransactionEngine::create_account(uint64_t account_id,
                                       int64_t initial_balance)
{
    account_balances_[account_id] = initial_balance;
}

Result TransactionEngine::transfer(
    uint64_t transaction_id,
    uint64_t source_account,
    uint64_t destination_account,
    int64_t amount,
    std::string_view idempotency_key)
{
    uint64_t time = nanosecondNow();

    // cant transfer money to yourself
    if (source_account == destination_account)
    {
        return {false, Error::InvalidTransfer};
    }

    // amount should be greater than 0
    if (amount <= 0)
    {
        return {false, Error::InvalidAmount};
    }

    if (idempotency_key.empty())
    {
        return {false, Error::InvalidIdempotencyKey};
    }

    // accounts should exist
    if (!account_balances_.contains(source_account) || !account_balances_.contains(destination_account))
    {
        return {false, Error::AccountNotFound};
    }

    auto transaction_it = transactions_.find(transaction_id);
    auto idempotency_key_it = idempotency_key_to_transaction_id_.find(idempotency_key);

    if (transaction_it != transactions_.end())
    {
        const auto &transaction = transaction_it->second;

        if (idempotency_key_it == idempotency_key_to_transaction_id_.end() || idempotency_key_it->second != transaction_id)
        {
            return {false, Error::InvalidIdempotencyKey};
        }

        if (

            transaction.amount != amount ||
            transaction.source_account != source_account ||
            transaction.destination_account != destination_account)
        {
            return {false, Error::InconsistentTransfer};
        }

        // transaction already exists and handeled
        return {true};
    }
    else
    {
        // check duplicate idempotency_key
        if (idempotency_key_it != idempotency_key_to_transaction_id_.end())
        {
            return {false, Error::IdempotencyKeyReused};
        }
    }

    auto &source_balance = account_balances_.at(source_account);
    auto &destination_balance = account_balances_.at(destination_account);

    // check if sufficient amount on source account
    if (source_balance < amount)
    {
        return {false, Error::InsufficientFunds};
    }

    // update balances of accounts
    source_balance = source_balance - amount;
    destination_balance = destination_balance + amount;

    // store transaction
    transactions_[transaction_id] = Transaction{
        transaction_id,
        source_account,
        destination_account,
        amount,
        time,
        TransactionStatus::Completed,
    };
    idempotency_key_to_transaction_id_[idempotency_key] = transaction_id;

    return {true};
}

TransactionStatus TransactionEngine::status(
    uint64_t transaction_id) const
{
    auto transaction_it = transactions_.find(transaction_id);
    if (transaction_it == transactions_.end())
    {
        return TransactionStatus::Unknown;
    }

    return transaction_it->second.status;
}

int64_t TransactionEngine::balance(uint64_t account_id) const
{
    return this->account_balances_.at(account_id);
}

Transaction TransactionEngine::get_transaction(
    uint64_t transaction_id) const
{
    return this->transactions_.at(transaction_id);
}