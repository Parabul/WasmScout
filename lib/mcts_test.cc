#include "lib/mcts.h"

#include <iostream>

#include "gtest/gtest.h"

namespace scout
{

    TEST(AverageValueTest, ZeroValueForBothPlayers)
    {
        AverageValue averageValue;
        EXPECT_FLOAT_EQ(averageValue.getValue(Player::ONE), 0.0f);
        EXPECT_FLOAT_EQ(averageValue.getValue(Player::TWO), 0.0f);
    }

    TEST(AverageValueTest, NegativeValueForTheOpponent)
    {
        AverageValue averageValue(1.0f, 1);
        EXPECT_FLOAT_EQ(averageValue.getValue(Player::ONE), 1.0f);
        EXPECT_FLOAT_EQ(averageValue.getValue(Player::TWO), -1.0f);
    }

    TEST(AverageValueTest, AddWinners)
    {

        AverageValue averageValue(2.0f, 2);

        averageValue.addWinner(Player::ONE);
        averageValue.addWinner(Player::TWO);
        averageValue.addWinner(Player::NONE);

        AverageValue averageValueExpected(2.0f, 5);

        EXPECT_EQ(averageValue, averageValueExpected);

        EXPECT_FLOAT_EQ(averageValue.getValue(Player::ONE), 0.4f);
        EXPECT_FLOAT_EQ(averageValue.getValue(Player::TWO), -0.4f);
    }

    TEST(AverageValueTest, AddingOppositeValueReturnsZero)
    {
        AverageValue averageValue(1.0f, 1);
        averageValue.addWinner(Player::TWO);

        EXPECT_FLOAT_EQ(averageValue.getValue(Player::ONE), 0.0f);
        EXPECT_FLOAT_EQ(averageValue.getValue(Player::TWO), 0.0f);
    }

    // --- Tests for StateEvaluation ---
    // These tests are self-contained and don't depend on the game state.

    TEST(StateEvaluationTest, ConstructorInitializesCorrectly)
    {
        const int numMoves = 7;
        StateEvaluation eval(numMoves);

        EXPECT_EQ(eval.getNumberOfMoves(), numMoves);
        EXPECT_EQ(eval.getValue(), 0.0f);
        ASSERT_EQ(eval.getPolicy().size(), numMoves);
        for (float p : eval.getPolicy())
        {
            EXPECT_EQ(p, 0.0f);
        }
    }

    TEST(StateEvaluationTest, GettersAndSettersWork)
    {
        StateEvaluation eval(3);

        eval.setValue(0.85f);
        EXPECT_FLOAT_EQ(eval.getValue(), 0.85f);

        // Check both const and non-const versions of getPolicy()
        eval.getPolicy() = {0.5f, 0.2f, 0.3f};
        const auto &policy = eval.getPolicy();
        EXPECT_FLOAT_EQ(policy[0], 0.5f);
        EXPECT_FLOAT_EQ(policy[1], 0.2f);
        EXPECT_FLOAT_EQ(policy[2], 0.3f);
    }

    TEST(StateEvaluationTest, EqualityOperatorWorks)
    {
        StateEvaluation eval1(3);
        eval1.setValue(0.5f);
        eval1.getPolicy() = {0.1f, 0.2f, 0.7f};

        StateEvaluation eval2(3);
        eval2.setValue(0.5f);
        eval2.getPolicy() = {0.1f, 0.2f, 0.7f};

        StateEvaluation eval3(3); // Different value
        eval3.setValue(-0.5f);
        eval3.getPolicy() = {0.1f, 0.2f, 0.7f};

        StateEvaluation eval4(3); // Different policy
        eval4.setValue(0.5f);
        eval4.getPolicy() = {0.2f, 0.1f, 0.7f};

        StateEvaluation eval5(4); // Different size

        EXPECT_EQ(eval1, eval2);
        // EXPECT_NE(eval1, eval3);
        // EXPECT_NE(eval1, eval4);
        // EXPECT_NE(eval1, eval5);
    }

    // --- Tests for AverageValue ---
    // These tests use the real Player enum from game.h.

    TEST(AverageValueTest, DefaultConstructor)
    {
        AverageValue avg;
        // With 0 support, value should be 0 to avoid division by zero.
        EXPECT_EQ(avg.getValue(Player::ONE), 0.0f);
        EXPECT_EQ(avg.getValue(Player::TWO), 0.0f);
    }

    TEST(AverageValueTest, ParameterizedConstructor)
    {
        AverageValue avg(2.5f, 5);
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), 0.5f);
        EXPECT_FLOAT_EQ(avg.getValue(Player::TWO), -0.5f);
    }

    TEST(AverageValueTest, FromEvaluation)
    {
        AverageValue avg;

        avg.fromEvaluation(Player::ONE, 0.8f);
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), 0.8f);

        // An evaluation from P2's perspective is inverted to be stored from P1's view.
        avg.fromEvaluation(Player::TWO, 0.6f);
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), -0.6f);
        EXPECT_FLOAT_EQ(avg.getValue(Player::TWO), 0.6f);
    }

    TEST(AverageValueTest, AddWinner)
    {
        AverageValue avg;

        avg.addWinner(Player::ONE); // P1_Val: 1, Support: 1
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), 1.0f);

        avg.addWinner(Player::TWO); // P1_Val: 1-1=0, Support: 2
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), 0.0f);

        avg.addWinner(Player::NONE); // P1_Val: 0, Support: 3
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), 0.0f);

        avg.addWinner(Player::ONE); // P1_Val: 1, Support: 4
        EXPECT_FLOAT_EQ(avg.getValue(Player::ONE), 0.25f);
        EXPECT_FLOAT_EQ(avg.getValue(Player::TWO), -0.25f);
    }

    TEST(AverageValueTest, AddAndPlusEqualsOperators)
    {
        AverageValue avg1(2.0f, 4); // Value = 0.5
        AverageValue avg2(1.0f, 1); // Value = 1.0

        // Test add() method
        AverageValue sum1 = avg1;
        sum1.add(avg2); // P1_Val: 2+1=3, Support: 4+1=5
        EXPECT_FLOAT_EQ(sum1.getValue(Player::ONE), 0.6f);

        // Test operator+=
        AverageValue sum2 = avg1;
        sum2 += avg2;
        EXPECT_FLOAT_EQ(sum2.getValue(Player::ONE), 0.6f);

        // Both methods should yield the same result
        EXPECT_EQ(sum1, sum2);
    }

    // --- Tests for ZeroValueUniformEvaluator ---
    // This test uses the real GameState to construct TreeNodes.

    TEST(ZeroValueUniformEvaluatorTest, OperatorAppliesCorrectEvaluation)
    {
        const int numMoves = GameState::NUM_MOVES;
        auto state1 = std::make_unique<GameState>();
        auto state2 = std::make_unique<GameState>();
        TreeNode node1(std::move(state1), numMoves);
        TreeNode node2(std::move(state2), numMoves);

        std::vector<TreeNode *> nodes = {&node1, &node2};

        ZeroValueUniformEvaluator evaluator(numMoves);
        evaluator(nodes);

        // Check node 1
        EXPECT_EQ(node1.evaluation().getValue(), 0.0f);
        ASSERT_EQ(node1.evaluation().getPolicy().size(), numMoves);
        for (float p : node1.evaluation().getPolicy())
        {
            EXPECT_FLOAT_EQ(p, 1.0f / numMoves);
        }

        // Check node 2
        EXPECT_EQ(node2.evaluation().getValue(), 0.0f);
        ASSERT_EQ(node2.evaluation().getPolicy().size(), numMoves);
        for (float p : node2.evaluation().getPolicy())
        {
            EXPECT_FLOAT_EQ(p, 1.0f / numMoves);
        }
    }

    // --- Tests for TreeNode ---
    // This test fixture uses the real GameState and Outcomes classes from game.h.
    class TreeNodeTest : public ::testing::Test
    {
    protected:
        const int numMoves_ = GameState::NUM_MOVES;
        std::unique_ptr<TreeNode> node_;

        void SetUp() override
        {
            // The TreeNode takes ownership of the state pointer.
            // A default-constructed GameState is a valid, non-terminal starting state.
            node_ = std::make_unique<TreeNode>(std::make_unique<GameState>(), numMoves_);
        }
    };

    TEST_F(TreeNodeTest, ConstructorInitializesState)
    {
        EXPECT_FALSE(node_->isInitialized());
        EXPECT_TRUE(node_->isLeaf());
        EXPECT_EQ(node_->getVisits(), 0);
        EXPECT_EQ(node_->evaluation().getNumberOfMoves(), numMoves_);
        EXPECT_EQ(node_->getChildStates().size(), 9);
        EXPECT_EQ(node_->state().getCurrentPlayer(), Player::ONE);
    }

    TEST_F(TreeNodeTest, UpdateModifiesStatsCorrectly)
    {
        AverageValue val1(0.5f, 1);
        node_->update(Player::ONE, val1);

        EXPECT_EQ(node_->getVisits(), 1);

        Outcomes expected_outcomes;
        expected_outcomes.addWinner(Player::ONE);
        EXPECT_EQ(node_->getOutcomes(), expected_outcomes);
        EXPECT_FLOAT_EQ(node_->getAverageValue().getValue(Player::ONE), 0.5f);

        AverageValue val2(-1.0f, 1);
        node_->update(Player::TWO, val2); // total val: 0.5 - 1.0 = -0.5, total support: 2

        EXPECT_EQ(node_->getVisits(), 2);
        expected_outcomes.addWinner(Player::TWO);
        EXPECT_EQ(node_->getOutcomes(), expected_outcomes);
        EXPECT_FLOAT_EQ(node_->getAverageValue().getValue(Player::ONE), -0.25f);
    }

    TEST_F(TreeNodeTest, InitChildrenOnNonTerminalNode)
    {
        // NOTE: We assume the TreeNode implementation correctly calls state->isGameOver()
        // instead of the non-existent state->isTerminal().
        bool evaluator_called = false;
        Evaluator mock_evaluator =
            [&](const std::vector<TreeNode *> &nodes)
        {
            evaluator_called = true;
            ASSERT_EQ(nodes.size(), numMoves_);
            float val = 0.5f;
            // Give each new child a unique evaluation for testing.
            for (auto *child_node : nodes)
            {
                child_node->evaluation().setValue(val);
                val += 0.1f; // e.g., 0.5, 0.6, 0.7...
            }
        };

        auto result = node_->initChildren(mock_evaluator);

        // Verify node state changes
        EXPECT_TRUE(evaluator_called);
        EXPECT_TRUE(node_->isInitialized());
        EXPECT_FALSE(node_->isLeaf());
        ASSERT_EQ(node_->getChildStates().size(), numMoves_);

        // Verify that child nodes have the correct next player state.
        // The default state's current player is ONE, so all children should be TWO.
        for (const auto &child : node_->getChildStates())
        {
            EXPECT_EQ(child->state().getCurrentPlayer(), Player::TWO);
        }

        // The evaluator returns values from the perspective of the children's
        // current player (Player::TWO). AverageValue should store them negated.
        float expected_p1_value_sum = 0.0f;
        float current_eval = 0.5f;
        for (int i = 0; i < numMoves_; ++i)
        {
            expected_p1_value_sum += -current_eval; // Negate since it's opponent's turn
            current_eval += 0.1f;
        }

        ASSERT_TRUE(result.has_value());
        EXPECT_FLOAT_EQ(result->getValue(Player::ONE), expected_p1_value_sum / numMoves_);

        // Calling a second time should be a no-op
        auto second_result = node_->initChildren(mock_evaluator);
        EXPECT_FALSE(second_result.has_value());
    }

    TEST_F(TreeNodeTest, EncodeOnInternalNode)
    {
        // 1. Initialize children to make the node "internal"
        Evaluator dummy_evaluator = [](const std::vector<TreeNode *> &) {};
        node_->initChildren(dummy_evaluator);
        node_->getAverageValue().fromEvaluation(Player::ONE, 0.9f); // Set parent value

        // 2. "Visit" the children to give them non-zero stats
        node_->getChildStates()[0]->update(Player::ONE, AverageValue());
        node_->getChildStates()[0]->update(Player::ONE, AverageValue());
        node_->getChildStates()[1]->update(Player::TWO, AverageValue());
        node_->getChildStates()[2]->update(Player::ONE, AverageValue());

        // 3. Encode and verify the [value, policy...] vector
        auto encoded = node_->encode();
        ASSERT_EQ(encoded.size(), 1 + numMoves_);
        EXPECT_FLOAT_EQ(encoded[0], 0.9f);        // Parent value
        EXPECT_FLOAT_EQ(encoded[1], 2.0f / 4.0f); // Policy for child 0 (p = visits/total)
        EXPECT_FLOAT_EQ(encoded[2], 1.0f / 4.0f); // Policy for child 1
        EXPECT_FLOAT_EQ(encoded[3], 1.0f / 4.0f); // Policy for child 2
        EXPECT_FLOAT_EQ(encoded[4], 0.0f / 4.0f); // Policy for child 3 (0 visits)
    }

    // Define a tolerance for floating-point comparisons, similar to the Java test
    constexpr float TOLERANCE = 1e-6f;

    // Define the test case using the Google Test framework
    TEST(OnnxEvaluatorTest, EvaluateNinePebbles)
    {
        scout::OnnxEvaluator evaluator;

        auto game1 = std::make_unique<scout::GameState>();
        auto game2 = game1->move(6); // Create the second state by making a move

        auto node1 = std::make_unique<scout::TreeNode>(std::move(game1), scout::GameState::NUM_MOVES);
        auto node2 = std::make_unique<scout::TreeNode>(std::move(game2), scout::GameState::NUM_MOVES);

        evaluator({node1.get(), node2.get()});

        ASSERT_NEAR(node1->evaluation().getValue(), -0.004695917f, TOLERANCE);

        // Check the policy vector
        const std::vector<float> expected_policy1 = {
            0.04805885, 0.22749093, 0.013468214, 0.17588913, 0.23200665, 0.1259972, 0.022932446, 0.0069662756, 0.14719029};
        const auto &actual_policy1 = node1->evaluation().getPolicy();

        ASSERT_EQ(actual_policy1.size(), expected_policy1.size());
        for (size_t i = 0; i < actual_policy1.size(); ++i)
        {
            ASSERT_NEAR(actual_policy1[i], expected_policy1[i], TOLERANCE);
        }

        // 6. Assert the results for the second node
        // Check the state value
        ASSERT_NEAR(node2->evaluation().getValue(), 0.056121465f, TOLERANCE);

        // Check the policy vector
        const std::vector<float> expected_policy2 = {
            0.089444205, 0.010340182, 0.08174314, 0.5893696, 0.13320167, 1.2439788E-4, 0.07818858, 0.011549589, 0.0060386327};
        const auto &actual_policy2 = node2->evaluation().getPolicy();

        ASSERT_EQ(actual_policy2.size(), expected_policy2.size());
        for (size_t i = 0; i < actual_policy2.size(); ++i)
        {
            ASSERT_NEAR(actual_policy2[i], expected_policy2[i], TOLERANCE);
        }
    }

    TEST(MonteCarloTreeSearchTest, ExpandsShortestGameMultipleTimesWithOnnx)
    {
        // --- 1. ARRANGE: Set up the MCTS components ---

        // 🧠 Initialize the ONNX evaluator.
        // NOTE: You must provide a valid path to your .onnx model file for this test to pass.
        scout::OnnxEvaluator onnx_evaluator;

        // 🧭 Initialize the expansion strategy.
        scout::PredictiveUpperConfidenceBound pucb_strategy;

        // 🌲 Create the main MCTS orchestrator.
        scout::MonteCarloTreeSearch mcts(std::ref(pucb_strategy), std::ref(onnx_evaluator));

        // 🌳 Create the root node for a new game.
        auto root_state = std::make_unique<scout::GameState>();
        root_state = root_state->move(8)->move(1)->move(7)->move(3)->move(6)->move(3)->move(4)->move(1)->move(8)->move(8);

        // 1. 98 (10), 22 (10)
        // 2. 87 (22), 46 (20)
        // 3. 76 (36), 45
        // 4. 55 (52), 25
        // 5. 93 (68), 91!
        // 6. 91 (84)

        std::cout << root_state->toString();

        auto root_node = scout::TreeNode(std::move(root_state), scout::GameState::NUM_MOVES);

        const int num_expansions = 10000;

        // --- 2. ACT: Run the MCTS expansion loop ---

        // Perform the search for a fixed number of iterations.
        for (int i = 0; i < num_expansions; ++i)
        {
            mcts.expand(&root_node);
        }

        // --- 3. ASSERT: Verify the results ---

        // ✅ The root node should have been visited in every single expansion.
        ASSERT_EQ(root_node.getVisits(), num_expansions);

        // ✅ The total number of outcomes (win/loss/tie) recorded should also match.
        ASSERT_EQ(root_node.getOutcomes().getTotalOutcomes(), num_expansions);

        auto encoded = root_node.encode();
        std::cout << "Encoded: [";
        for (size_t i = 0; i < encoded.size(); ++i)
        {
            std::cout << encoded[i] << ",";
        }
        std::cout << "] ";

        const std::vector<float> expected_encoded = {0.8751, 0.014, 0.014, 0.0164, 0.0149, 0.0159, 0.0164, 0.013, 0.03, 0.86};

        ASSERT_EQ(encoded.size(), expected_encoded.size());
        for (size_t i = 0; i < expected_encoded.size(); ++i)
        {
            ASSERT_NEAR(encoded[i], expected_encoded[i], 0.01);
        }
    }
} // namespace scout