#pragma once

enum class Error
{
    None,
    AccountNotFound,
    InvalidTransfer,
    InconsistentTransfer,
    TransactionNotFound,
    InvalidAmount,
    InvalidIdempotencyKey,
    InsufficientFunds,
    InvalidState,
    DuplicateTransaction,
    IdempotencyKeyReused
};

struct Result
{
    bool success;
    Error error;
};