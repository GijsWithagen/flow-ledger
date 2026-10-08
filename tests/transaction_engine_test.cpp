#include <gtest/gtest.h>

#include "flow-ledger/transaction_engine.hpp"

TEST(TransactionEngineTest, CannotTransferToSelf)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        100,
        50,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InvalidTransfer);
    EXPECT_EQ(engine.balance(100), 100);
    EXPECT_EQ(engine.balance(101), 100);
}

TEST(TransactionEngineTest, CannotTransferNeggativeAmount)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        101,
        -1,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InvalidAmount);
    EXPECT_EQ(engine.balance(100), 100);
    EXPECT_EQ(engine.balance(101), 100);
}

TEST(TransactionEngineTest, CannotTransferZeroAmount)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        101,
        -1,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InvalidAmount);
    EXPECT_EQ(engine.balance(100), 100);
    EXPECT_EQ(engine.balance(101), 100);
}

TEST(TransactionEngineTest, CanTransfer)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
    EXPECT_EQ(engine.get_transaction(1).status, TransactionStatus::Completed);
}

TEST(TransactionEngineTest, WillTransferOnceWithIdempotencyKey)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result.success);

    auto result2 = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result2.success);

    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
}

TEST(TransactionEngineTest, CannotTransferInvalidIdempotencyKey)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result1 = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result1.success);

    auto result = engine.transfer(
        1,
        100,
        101,
        10,
        "other-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InvalidIdempotencyKey);
    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
}

TEST(TransactionEngineTest, CannotTransferIdempotencyKeyReused)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result1 = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result1.success);

    auto result = engine.transfer(
        2,
        100,
        101,
        10,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::IdempotencyKeyReused);
    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
}

TEST(TransactionEngineTest, CannotTransferInconsistentTransfer)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result1 = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result1.success);

    auto result = engine.transfer(
        1,
        100,
        101,
        11,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InconsistentTransfer);
    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
}

TEST(TransactionEngineTest, CannotTransferInconsistentAmount)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result1 = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result1.success);

    auto result = engine.transfer(
        1,
        100,
        101,
        11,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InconsistentTransfer);
    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
}

TEST(TransactionEngineTest, CannotTransferInconsistentSource)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);
    engine.create_account(102, 100);

    auto result1 = engine.transfer(
        1,
        102,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result1.success);

    auto result = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InconsistentTransfer);
    EXPECT_EQ(engine.balance(102), 90);
    EXPECT_EQ(engine.balance(101), 110);
    EXPECT_EQ(engine.balance(100), 100);
}

TEST(TransactionEngineTest, CannotTransferInconsistentDestination)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);
    engine.create_account(102, 100);

    auto result1 = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_TRUE(result1.success);

    auto result = engine.transfer(
        1,
        100,
        102,
        10,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InconsistentTransfer);
    EXPECT_EQ(engine.balance(100), 90);
    EXPECT_EQ(engine.balance(101), 110);
    EXPECT_EQ(engine.balance(102), 100);
}

TEST(TransactionEngineTest, CannotTransferInsufficientFunds)
{
    TransactionEngine engine;
    engine.create_account(100, 9);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        101,
        10,
        "test-key");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InsufficientFunds);
    EXPECT_EQ(engine.balance(100), 9);
    EXPECT_EQ(engine.balance(101), 100);
}

TEST(TransactionEngineTest, CannotTransferEmptyIdempotencyKey)
{
    TransactionEngine engine;
    engine.create_account(100, 100);
    engine.create_account(101, 100);

    auto result = engine.transfer(
        1,
        100,
        101,
        10,
        "");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, Error::InvalidIdempotencyKey);
    EXPECT_EQ(engine.balance(100), 100);
    EXPECT_EQ(engine.balance(101), 100);
}