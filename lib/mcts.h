#ifndef WASM_SCOUT_LIB_MCTS_H
#define WASM_SCOUT_LIB_MCTS_H

#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "lib/game.h"
#include "onnxruntime/core/session/onnxruntime_cxx_api.h"

namespace scout
{

    // Represents an evaluation of a game state as viewed from the current player.
    class StateEvaluation
    {
    public:
        // Constructor initializes the policy vector and sets the value to 0.
        explicit StateEvaluation(int numberOfMoves);

        // --- Getters and Setters ---

        // Returns the number of possible moves (the size of the policy vector).
        int getNumberOfMoves() const;

        // Gets the value of the state [-1, 1].
        float getValue() const;

        // Sets the value of the state.
        void setValue(float value);

        // Returns a constant reference to the policy vector (for reading).
        const std::vector<float> &getPolicy() const;

        // Returns a mutable reference to the policy vector (for modifying).
        std::vector<float> &getPolicy();

        // --- Utilities ---

        // Creates a string representation of the object.
        std::string toString() const;

        // --- Operators (Idiomatic C++) ---

        // Checks for equality with another StateEvaluation object.
        bool operator==(const StateEvaluation &other) const;

    private:
        // Policy outcomes for each move. A vector of floats [0, 1] that sum to 1.
        std::vector<float> _policy;

        // A value in the range [-1, 1] indicating how favorable the state is.
        float _value;
    };

    // Enables printing to std::ostream (e.g., std::cout << my_eval;).
    std::ostream &operator<<(std::ostream &os, const StateEvaluation &eval);

    class AverageValue
    {
    public:
        // --- Constructors ---

        // Default constructor
        AverageValue();

        // Parameterized constructor
        AverageValue(float playerOneValue, int support);

        // --- Public Methods ---

        // Gets the calculated average value for a specific player.
        float getValue(Player player) const;

        // Sets the state from a single evaluation and returns a reference to self.
        AverageValue &fromEvaluation(Player currentPlayer, float evaluatedValue);

        // Adds another AverageValue's data to this one (in-place).
        // This is the direct translation of the Java method.
        AverageValue &add(const AverageValue &other);

        // Adds a win/loss/tie to the total and returns a reference to self.
        AverageValue &addWinner(Player player);

        // Creates a string representation of the object.
        std::string toString() const;

        // --- Operator Overloads (Idiomatic C++) ---

        // Overloads the += operator; more idiomatic than the add() method.
        AverageValue &operator+=(const AverageValue &other);

        // Overloads the == operator for equality checks.
        bool operator==(const AverageValue &other) const;

    private:
        // The total observed value, always stored from Player ONE's perspective.
        float _playerOneValue = 0.0f;
        // The number of samples (games, evaluations) observed.
        int _support = 0;
    };

    // Forward-declare TreeNode to avoid include cycles
    class TreeNode;

    // The Evaluator now works directly with TreeNode pointers.
    using Evaluator = std::function<void(const std::vector<TreeNode *> &)>;

    /**
     * @brief A baseline evaluator that assigns a zero value and a uniform policy.
     *
     * For any non-terminal game state, this evaluator sets the state's value to 0
     * and distributes the policy probability evenly among all possible moves.
     */
    class ZeroValueUniformEvaluator
    {
    public:
        // Constructor calculates the uniform policy value based on the total number of moves.
        explicit ZeroValueUniformEvaluator(int numMoves);

        // The call operator applies the evaluation logic to a vector of nodes.
        void operator()(const std::vector<TreeNode *> &nodes) const;

    private:
        float _policyValue;
    };

    /**
     * @brief Represents a single node in the Monte Carlo Search Tree.
     */
    class TreeNode
    {
    public:
        // Constructor takes ownership of the GameState object.
        TreeNode(std::unique_ptr<GameState> state, int numMoves);

        // Updates the node's statistics from a simulation result.
        void update(Player winner, const AverageValue &averageValue);

        /**
         * @brief Initializes child states and evaluates them using the provided evaluator.
         * @return An optional AverageValue representing the combined value of all new children.
         */
        std::optional<AverageValue> initChildren(const Evaluator &evaluator);

        /**
         * @brief Encodes the node's stats into a format for ML training.
         * The first element is the node's value, followed by the normalized visit counts
         * of its children (policy).
         * @return A vector of floats representing the value and policy.
         */
        std::vector<float> encode() const;

        // --- Getters and State Checks ---
        const GameState &state() const;
        StateEvaluation &evaluation();
        const StateEvaluation &evaluation() const;
        const std::vector<std::unique_ptr<TreeNode>> &getChildStates() const;
        const Outcomes &getOutcomes() const;
        AverageValue &getAverageValue();
        bool isInitialized() const;
        bool isLeaf() const;
        int getVisits() const;
        std::string toString() const;

    private:
        std::unique_ptr<GameState> _state;
        StateEvaluation _evaluation;
        AverageValue _averageValue;
        Outcomes _outcomes;
        // Child nodes are owned by this node.
        std::vector<std::unique_ptr<TreeNode>> _childStates;
        bool _initialized = false;
    };

    /**
     * @brief An evaluator that uses an ONNX model to perform batch inference on TreeNodes.
     *
     * This class is designed as a functor, meaning it can be used wherever a
     * scout::Evaluator (std::function) is expected.
     */
    class OnnxEvaluator
    {
    public:
        /**
         * @brief Constructs the evaluator by loading an ONNX model from a file.
         * @param model_path The filesystem path to the .onnx model file.
         */
        explicit OnnxEvaluator();

        /**
         * @brief The call operator that performs the evaluation.
         * This makes the object callable like a function, matching the scout::Evaluator signature.
         * @param nodes A vector of TreeNode pointers to be evaluated in a single batch.
         */
        void operator()(const std::vector<TreeNode *> &nodes);

    private:
        // ONNX Runtime environment and session.
        Ort::Env env_;
        Ort::Session session_;
        Ort::MemoryInfo memory_info_;

        // Pre-allocated buffer for the input batch and its shape.
        std::vector<float> batch_input_;
        std::vector<int64_t> input_shape_;

        // Game constants derived from GameState.
        const size_t num_features_ = GameState::NUM_FEATURES;
        const size_t num_moves_ = GameState::NUM_MOVES;

        // Constants for model input/output tensor names.
        static constexpr const char *INPUT_NAME = "input_1";
        static constexpr const char *VALUE_OUTPUT_NAME = "value_output";
        static constexpr const char *POLICY_OUTPUT_NAME = "policy_output";
    };

    using ExpansionStrategy = std::function<int(const TreeNode &)>;

    /**
     * @brief A functor implementing the PUCT (Predictor + UCT) algorithm.
     *
     * This object can be passed as a scout::ExpansionStrategy.
     */
    class PredictiveUpperConfidenceBound
    {
    public:
        explicit PredictiveUpperConfidenceBound();

        /**
         * @brief The call operator that makes this object a functor.
         * @param treeNode The current, initialized node whose children are considered.
         * @return The index of the child node to move to.
         */
        int operator()(const TreeNode &treeNode);

    private:
        std::vector<double> sampleDirichlet();

        static constexpr float EXPLORATION_WEIGHT = 4.0f;
        static constexpr float NOISE_WEIGHT = 0.25f;

        size_t num_moves_ = GameState::NUM_MOVES;
        std::mt19937 random_generator_;
        std::gamma_distribution<double> gamma_distribution_;
    };

    class MonteCarloTreeSearch
    {
    public:
        // The constructor now takes the strategy by value
        MonteCarloTreeSearch(ExpansionStrategy strategy, Evaluator evaluator);

        void expand(TreeNode *rootNode);

    private:
        // The member is now a std::function, not a unique_ptr
        ExpansionStrategy expansion_strategy_;
        Evaluator evaluator_;
    };

} // namespace scout

#endif // WASM_SCOUT_LIB_MCTS_H