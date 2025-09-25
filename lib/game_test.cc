#include <iostream>
#include <map>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lib/game.h" // Include the header for the class we are testing.

namespace scout
{
    // GTest matchers for cleaner vector comparisons with a tolerance.
    using ::testing::FloatNear;
    using ::testing::Pointwise;

    const float EPSILON = 1e-6;

    TEST(GameStateTest, ShortestGame)
    {
        auto root_state = std::make_unique<scout::GameState>();
        auto game_over_state = root_state->move(8)->move(1)->move(7)->move(3)->move(6)->move(3)->move(4)->move(1)->move(8)->move(8)->move(8);

        // 1. 98 (10), 22 (10)
        // 2. 87 (22), 46 (20)
        // 3. 76 (36), 45
        // 4. 55 (52), 25
        // 5. 93 (68), 91!
        // 6. 91 (84)

        std::cout << game_over_state->toString() << std::endl;

        EXPECT_TRUE(game_over_state->isGameOver());
        EXPECT_EQ(game_over_state->getWinner(), Player::ONE);
    }

    // The test suite is named GameStateTest, similar to the Java class name.
    // Each TEST macro defines a test case, equivalent to a method with @Test.
    TEST(GameStateTest, StateDefaultConstructor)
    {
        GameState state;

        EXPECT_EQ(state.getCurrentPlayer(), Player::ONE);
        EXPECT_FALSE(state.getWinner().has_value()); // Java: .isNull()
        EXPECT_FALSE(state.isGameOver());
        for (int i = 0; i < 9; ++i)
        {
            EXPECT_TRUE(state.isMoveAllowed(i));
        }

        std::vector<float> expected_encode = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0.11111111f, 0.11111111f, 0.11111111f, 0.11111111f, 0.11111111f,
            0.11111111f, 0.11111111f, 0.11111111f, 0.11111111f,
            0.11111111f, 0.11111111f, 0.11111111f, 0.11111111f, 0.11111111f,
            0.11111111f, 0.11111111f, 0.11111111f, 0.11111111f,
            0, 0,
            0.0f, 0.12345679f, 0.12345679f, 0.12345679f, 0.12345679f,
            0.12345679f, 0.12345679f, 0.12345679f, 0.12345679f};

        EXPECT_THAT(state.encode(), Pointwise(FloatNear(EPSILON), expected_encode));

        std::cout << state.toString() << std::endl;

        auto newState = state.move(8);

        std::cout << newState->toString() << std::endl;
        EXPECT_EQ(newState->getCurrentPlayer(), Player::TWO);
        EXPECT_FALSE(newState->getWinner().has_value());
        EXPECT_FALSE(newState->isGameOver());
        for (int i = 0; i < 9; ++i)
        {
            if (i == 7)
            {
                EXPECT_FALSE(newState->isMoveAllowed(i));
            }
            else
            {
                EXPECT_TRUE(newState->isMoveAllowed(i));
            }
        }

        std::vector<float> expected_new_encode = {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.12345679,
            0.12345679, 0.12345679, 0.0, 0.11111111,
            0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.11111111,
            0.11111111, 0.11111111, 0.11111111, 0.012345679,
            0.0, 0.12345679,
            0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.12345679,
            0.12345679, 0.12345679, 0.0, 0.12345679};

        EXPECT_THAT(newState->encode(), Pointwise(FloatNear(EPSILON), expected_new_encode));
    }

    TEST(GameStateTest, SparseValuesConstructor)
    {
        GameState state(
            Player::TWO,
            {{0, 1}, {1, 2}, {2, 3}, {11, 4}, {10, 5}, {9, 6}},
            24,
            21,
            12,
            GameState::SPECIAL_NOT_SET);

        std::cout << state.toString() << std::endl;

        std::vector<float> expected_encode = {

            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.074074075,
            0.061728396,
            0.049382716,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.037037037,
            0.024691358,
            0.012345679,
            0.25925925,
            0.2962963,
            -0.012345679,
            -0.012345679,
            -0.012345679,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0};

        EXPECT_THAT(state.encode(), Pointwise(FloatNear(EPSILON), expected_encode));
        EXPECT_FALSE(state.isGameOver());
        EXPECT_EQ(state.getCurrentPlayer(), Player::TWO);
    }

    TEST(GameStateTest, SparseValuesConstructorGameOverTie)
    {
        GameState state(Player::TWO, {}, 81, 81, 12, 4);

        std::cout << state.toString() << std::endl;
        std::vector<float> expected_encode = {
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

        EXPECT_THAT(state.encode(), Pointwise(FloatNear(EPSILON), expected_encode));
        EXPECT_TRUE(state.isGameOver());
        ASSERT_TRUE(state.getWinner().has_value());
        EXPECT_EQ(state.getWinner().value(), Player::NONE);
        EXPECT_EQ(state.getCurrentPlayer(), Player::TWO);
    }

    TEST(GameStateTest, SparseValuesConstructorGameOver)
    {
        GameState state(Player::TWO, {{0, 9}}, 81, 72, 12, 4);

        std::cout << state.toString() << std::endl;
        std::vector<float> expected_encode = {
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.11111111,
            0.8888889, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

        EXPECT_THAT(state.encode(), Pointwise(FloatNear(EPSILON), expected_encode));
        EXPECT_TRUE(state.isGameOver());
        ASSERT_TRUE(state.getWinner().has_value());
        EXPECT_EQ(state.getWinner().value(), Player::ONE);
        EXPECT_EQ(state.getCurrentPlayer(), Player::TWO);

        GameState stateInverse(Player::ONE, {{0, 9}}, 81, 72, 12, 4);

        std::cout << stateInverse.toString() << std::endl;
        std::vector<float> expected_inverse_encode = {
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.11111111,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            1.0, 0.8888889,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.012345679};

        EXPECT_THAT(stateInverse.encode(), Pointwise(FloatNear(EPSILON), expected_inverse_encode));
        EXPECT_FALSE(stateInverse.isGameOver()); // Based on game logic, moves are still possible
        EXPECT_FALSE(stateInverse.getWinner().has_value());
        EXPECT_EQ(stateInverse.getCurrentPlayer(), Player::ONE);
    }

    // It's good practice to group related tests into a test suite.
    // Here, we create a new suite for the estimator.
    TEST(GameStateMoveValuesEstimatorTest, EstimateMoveValuesForRoot)
    {
        // 1. Setup
        GameState root;
        GameStateMoveValuesEstimator estimator;
        std::vector<float> expected_values = {
            0.0f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f};

        // 2. Execution
        std::vector<float> actual_values = estimator.estimateMoveValues(root);

        // 3. Assertion
        // This checks that the actual and expected vectors match element-wise,
        // within the specified tolerance of 0.01.
        EXPECT_THAT(actual_values, Pointwise(FloatNear(EPSILON), expected_values));
    }

    TEST(GameStateMoveValuesEstimatorTest, EstimateMoveValuesForNonRoot)
    {
        // 1. Setup
        GameState root;
        auto state = root.move(6);
        std::cout << state->toString() << std::endl; // Equivalent to System.out.println

        GameStateMoveValuesEstimator estimator;
        std::vector<float> expected_values = {
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.12345679f,
            0.0f,
            0.12345679f,
            0.024691358f,
            0.0f};

        // 2. Execution
        std::vector<float> actual_values = estimator.estimateMoveValues(*state);

        // 3. Assertion
        EXPECT_THAT(actual_values, Pointwise(FloatNear(EPSILON), expected_values));
    }
}