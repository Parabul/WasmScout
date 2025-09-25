#include "lib/mcts.h"

#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <utility>

#include "lib/model.h"

namespace scout
{

    // Constructor uses a member initializer list for efficiency.
    StateEvaluation::StateEvaluation(int numberOfMoves)
        : _policy(numberOfMoves, 0.0f), // Creates a vector of size numberOfMoves, all initialized to 0.0f
          _value(0.0f)
    {
    }

    int StateEvaluation::getNumberOfMoves() const
    {
        return _policy.size();
    }

    float StateEvaluation::getValue() const
    {
        return _value;
    }

    void StateEvaluation::setValue(float value)
    {
        this->_value = value;
    }

    const std::vector<float> &StateEvaluation::getPolicy() const
    {
        return _policy;
    }

    std::vector<float> &StateEvaluation::getPolicy()
    {
        return _policy;
    }

    std::string StateEvaluation::toString() const
    {
        std::stringstream ss;
        ss << "StateEvaluation{value=" << _value << ", policy=[";

        // Efficiently stream the vector contents, separated by commas.
        if (!_policy.empty())
        {
            std::copy(_policy.begin(), _policy.end() - 1, std::ostream_iterator<float>(ss, ", "));
            ss << _policy.back();
        }

        ss << "]}";
        return ss.str();
    }

    bool StateEvaluation::operator==(const StateEvaluation &other) const
    {
        // std::vector's operator== handles the element-wise array comparison.
        return this->_value == other._value && this->_policy == other._policy;
    }

    std::ostream &operator<<(std::ostream &os, const StateEvaluation &eval)
    {
        os << eval.toString();
        return os;
    }

    AverageValue::AverageValue() : _playerOneValue(0.0f), _support(0) {}

    AverageValue::AverageValue(float playerOneValue, int support)
        : _playerOneValue(playerOneValue), _support(support) {}

    float AverageValue::getValue(Player player) const
    {
        if (_support == 0)
        {
            return 0.0f;
        }

        switch (player)
        {
        case Player::ONE:
            return _playerOneValue / _support;
        case Player::TWO:
            return -_playerOneValue / _support;
        default:
            // Player::NONE has no value, but throwing is consistent with Java
            throw std::invalid_argument("Player has no value associated with it.");
        }
    }

    AverageValue &AverageValue::fromEvaluation(Player currentPlayer, float evaluatedValue)
    {
        this->_support = 1;
        switch (currentPlayer)
        {
        case Player::ONE:
            this->_playerOneValue = evaluatedValue;
            break;
        case Player::TWO:
            // Store the value from Player ONE's perspective
            this->_playerOneValue = -evaluatedValue;
            break;
        default:
            throw std::invalid_argument("Cannot evaluate for the specified player.");
        }
        // Return a reference to the current object to allow chaining
        return *this;
    }

    AverageValue &AverageValue::add(const AverageValue &other)
    {
        // This method is kept for direct translation, but operator+= is preferred
        return (*this += other);
    }

    AverageValue &AverageValue::addWinner(Player player)
    {
        _support++;
        switch (player)
        {
        case Player::ONE:
            _playerOneValue++;
            break;
        case Player::TWO:
            _playerOneValue--;
            break;
        case Player::NONE:
            // A tie doesn't change the playerOneValue, only increments support
            break;
        }
        return *this;
    }

    std::string AverageValue::toString() const
    {
        std::stringstream ss;
        ss << "AverageValue{playerOneValue=" << _playerOneValue
           << ", support=" << _support << "}";
        return ss.str();
    }

    // --- Operator Overloads ---

    AverageValue &AverageValue::operator+=(const AverageValue &other)
    {
        this->_playerOneValue += other._playerOneValue;
        this->_support += other._support;
        return *this;
    }

    bool AverageValue::operator==(const AverageValue &other) const
    {
        // C++'s == on floats is equivalent to Java's Float.compare() == 0
        return this->_playerOneValue == other._playerOneValue &&
               this->_support == other._support;
    }

    // --- Free Function Implementation ---

    std::ostream &operator<<(std::ostream &os, const AverageValue &val)
    {
        os << val.toString();
        return os;
    }

    ZeroValueUniformEvaluator::ZeroValueUniformEvaluator(int numMoves)
        : _policyValue(0.0f)
    {
        // Prevent division by zero if numMoves is 0.
        if (numMoves > 0)
        {
            _policyValue = 1.0f / static_cast<float>(numMoves);
        }
    }

    void ZeroValueUniformEvaluator::operator()(const std::vector<TreeNode *> &nodes) const
    {
        for (TreeNode *node : nodes)
        {
            // Skip null pointers or nodes representing a completed game.
            if (node == nullptr || node->state().isGameOver())
            {
                continue;
            }

            // Set the value to a neutral 0.
            node->evaluation().setValue(0.0f);

            // Get a mutable reference to the policy vector.
            std::vector<float> &policy = node->evaluation().getPolicy();

            // Fill the entire policy vector with the pre-calculated uniform value.
            // This is the C++ equivalent of Java's Arrays.fill().
            std::fill(policy.begin(), policy.end(), _policyValue);
        }
    }

    TreeNode::TreeNode(std::unique_ptr<GameState> state, int numMoves)
        : _state(std::move(state)),
          _evaluation(numMoves)
    {
        _childStates.resize(numMoves); // Pre-allocate space, filled with nullptr
    }

    void TreeNode::update(Player winner, const AverageValue &averageValue)
    {
        this->_outcomes.addWinner(winner);
        this->_averageValue += averageValue; // Use the overloaded operator+=
    }

    std::optional<AverageValue> TreeNode::initChildren(const Evaluator &evaluator)
    {
        if (isInitialized())
        {
            return std::nullopt;
        }
        _initialized = true;

        int numberOfMoves = _evaluation.getNumberOfMoves();
        for (int move = 0; move < numberOfMoves; ++move)
        {
            if (!_state->isMoveAllowed(move))
            {
                continue;
            }
            // Create a new child node by making a move from the current state.
            _childStates[move] = std::make_unique<TreeNode>(_state->move(move), numberOfMoves);
        }

        // The evaluator processes the batch of new child nodes.
        // We create a vector of raw pointers to pass to it.
        std::vector<TreeNode *> child_raw_ptrs;
        child_raw_ptrs.reserve(_childStates.size());
        for (const auto &child_ptr : _childStates)
        {
            child_raw_ptrs.push_back(child_ptr.get());
        }
        evaluator(child_raw_ptrs);

        AverageValue childrenAverageValue;
        for (int move = 0; move < numberOfMoves; ++move)
        {
            if (_childStates[move] == nullptr)
            {
                continue;
            }
            TreeNode *childNode = _childStates[move].get();
            // Set the child's initial value from the evaluator's result.
            childNode->getAverageValue().fromEvaluation(
                childNode->state().getCurrentPlayer(),
                childNode->evaluation().getValue());
            childrenAverageValue += childNode->getAverageValue();
        }

        return childrenAverageValue;
    }

    std::vector<float> TreeNode::encode() const
    {
        if (isLeaf())
        {
            throw std::logic_error("Leaf node cannot be encoded.");
        }

        std::vector<float> outputs(_childStates.size() + 1);

        outputs[0] = _averageValue.getValue(_state->getCurrentPlayer());

        float totalVisits = 0;
        for (size_t move = 0; move < _childStates.size(); ++move)
        {
            if (_childStates[move] != nullptr)
            {
                outputs[move + 1] = static_cast<float>(_childStates[move]->getVisits());
                totalVisits += outputs[move + 1];
            }
        }

        if (totalVisits == 0)
        {
            throw std::logic_error("No visits found for non-leaf node: " + this->toString());
        }

        // Normalize visit counts to create a policy vector.
        for (size_t move = 0; move < _childStates.size(); ++move)
        {
            outputs[move + 1] /= totalVisits;
        }

        return outputs;
    }

    // --- Getters and State Checks ---

    const GameState &TreeNode::state() const { return *_state; }
    StateEvaluation &TreeNode::evaluation() { return _evaluation; }
    const StateEvaluation &TreeNode::evaluation() const { return _evaluation; }
    const std::vector<std::unique_ptr<TreeNode>> &TreeNode::getChildStates() const { return _childStates; }
    const Outcomes &TreeNode::getOutcomes() const { return _outcomes; }
    AverageValue &TreeNode::getAverageValue() { return _averageValue; }
    bool TreeNode::isInitialized() const { return _initialized; }
    bool TreeNode::isLeaf() const { return !_initialized || _state->isGameOver(); }
    int TreeNode::getVisits() const { return _outcomes.getTotalOutcomes(); }

    std::string TreeNode::toString() const
    {
        std::stringstream ss;
        ss << "TreeNode{state=" << _state->toString()
           << ", policy=" << _evaluation.toString()
           << ", averageValue=" << _averageValue.toString()
           << ", outcomes=" << _outcomes.toString()
           << ", initialized=" << std::boolalpha << _initialized << "}";
        return ss.str();
    }

    namespace
    { // Anonymous namespace for internal helper function
        // Helper to create an Ort::Env with specific threading options.
        Ort::Env create_env_with_threading_options()
        {
            Ort::ThreadingOptions tp_options;
            tp_options.SetGlobalInterOpNumThreads(1);
            tp_options.SetGlobalIntraOpNumThreads(1);
            return Ort::Env(tp_options, ORT_LOGGING_LEVEL_WARNING, "OnnxEvaluator");
        }
    }

    OnnxEvaluator::OnnxEvaluator() : env_(create_env_with_threading_options()),
                                     session_(env_, nine_pebbles_ort, nine_pebbles_ort_len, Ort::SessionOptions{nullptr}),
                                     memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU))
    {

        // Initialize the shape with a placeholder for the batch size (dim 0).
        input_shape_ = {1, static_cast<int64_t>(num_features_)};
    }

    void OnnxEvaluator::operator()(const std::vector<TreeNode *> &nodes)
    {
        if (nodes.empty())
        {
            return;
        }

        const size_t batch_size = nodes.size();

        // 1. Prepare the input batch tensor
        // Resize the flat input vector to hold all game states for this batch.
        batch_input_.resize(batch_size * num_features_);

        for (size_t i = 0; i < batch_size; ++i)
        {
            if (nodes[i] != nullptr)
            {
                // Get the encoded features from the node's game state.
                const auto &encoded_state = nodes[i]->state().encode();
                // Copy the features into the correct position in the batch vector.
                std::memcpy(batch_input_.data() + (i * num_features_), encoded_state.data(), num_features_ * sizeof(float));
            }
            else
            {
                // If a node is null, its corresponding part of the batch is zeroed out.
                std::memset(batch_input_.data() + (i * num_features_), 0, num_features_ * sizeof(float));
            }
        }

        // Update the input shape for the current batch size.
        input_shape_[0] = static_cast<int64_t>(batch_size);

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info_,
            batch_input_.data(),
            batch_input_.size(),
            input_shape_.data(),
            input_shape_.size());

        // 2. Run inference
        const char *input_names[] = {INPUT_NAME};
        const char *output_names[] = {VALUE_OUTPUT_NAME, POLICY_OUTPUT_NAME};

        try
        {
            auto output_tensors = session_.Run(
                Ort::RunOptions{nullptr},
                input_names, &input_tensor, 1,
                output_names, 2);

            // 3. Distribute the results back to the TreeNodes
            const float *value_ptr = output_tensors[0].GetTensorData<float>();
            const float *policy_ptr = output_tensors[1].GetTensorData<float>();

            for (size_t i = 0; i < batch_size; ++i)
            {
                if (nodes[i] == nullptr)
                {
                    continue;
                }

                // Get a mutable reference to the node's evaluation object.
                StateEvaluation &eval = nodes[i]->evaluation();

                // Set the scalar value.
                eval.setValue(value_ptr[i]);

                // Copy the policy vector from the output tensor.
                std::vector<float> &policy_vec = eval.getPolicy();
                std::memcpy(policy_vec.data(), policy_ptr + (i * num_moves_), num_moves_ * sizeof(float));
            }
        }
        catch (const Ort::Exception &e)
        {
            throw std::runtime_error("ONNX Runtime exception: " + std::string(e.what()));
        }
    }

    PredictiveUpperConfidenceBound::PredictiveUpperConfidenceBound()
        : // Seed the random number generator. For production, a better seed is recommended.
          random_generator_(std::random_device{}()),
          // A symmetric Dirichlet(1.0) sample can be generated from Gamma(1.0, 1.0).
          gamma_distribution_(1.0, 1.0)
    {
    }

    std::vector<double> PredictiveUpperConfidenceBound::sampleDirichlet()
    {
        std::vector<double> sample(num_moves_);
        double sum = 0.0;

        for (size_t i = 0; i < num_moves_; ++i)
        {
            sample[i] = gamma_distribution_(random_generator_);
            sum += sample[i];
        }

        // Normalize the samples to get the Dirichlet distribution
        if (sum > 0.0)
        {
            for (size_t i = 0; i < num_moves_; ++i)
            {
                sample[i] /= sum;
            }
        }

        return sample;
    }

    int PredictiveUpperConfidenceBound::operator()(const TreeNode &treeNode)
    {
        if (!treeNode.isInitialized())
        {
            throw std::logic_error("State node is not initialized!");
        }
        if (treeNode.isLeaf())
        {
            throw std::logic_error("State node is a leaf!");
        }

        float max_value = -std::numeric_limits<float>::max();
        int index_of_max = -1;

        const auto noises = sampleDirichlet();

        const double parent_visits_sqrt = std::sqrt(1.0 + treeNode.getVisits());
        const auto &children = treeNode.getChildStates();
        const auto &policy = treeNode.evaluation().getPolicy();

        for (size_t i = 0; i < children.size(); ++i)
        {
            const auto &child_state = children[i];
            if (child_state == nullptr)
            {
                continue;
            }

            const float prior_probability = policy[i];

            const float adjusted_probability =
                (prior_probability * (1.0f - NOISE_WEIGHT)) + (NOISE_WEIGHT * noises[i]);

            const float exploration = static_cast<float>(
                adjusted_probability * parent_visits_sqrt / (1.0 + child_state->getVisits()));

            const float exploitation =
                child_state->getAverageValue().getValue(treeNode.state().getCurrentPlayer());

            const float estimated_value = exploitation + EXPLORATION_WEIGHT * exploration;

            if (estimated_value > max_value)
            {
                max_value = estimated_value;
                index_of_max = i;
            }
        }

        if (index_of_max == -1)
        {
            throw std::runtime_error("Could not find any valid child states.");
        }

        return index_of_max;
    }

    MonteCarloTreeSearch::MonteCarloTreeSearch(ExpansionStrategy strategy, Evaluator evaluator)
        : expansion_strategy_(std::move(strategy)),
          evaluator_(std::move(evaluator)) {}

    void MonteCarloTreeSearch::expand(TreeNode *currentNode)
    {
        if (!currentNode)
            return;

        // The backpropagation path stores the node and the value of its immediate children.
        std::deque<std::pair<TreeNode *, std::optional<AverageValue>>> backprop_stack;

        int expansions = 0;
        // 1. SELECTION: Traverse the tree until a leaf node or unexpanded node is found.
        while (!currentNode->state().isGameOver() && expansions++ < 200)
        {
            // 2. EXPANSION: If the node is unvisited, initialize its children.
            std::optional<AverageValue> child_value = currentNode->initChildren(evaluator_);

            backprop_stack.emplace_back(currentNode, child_value);

            // If child_value has a value, it means we just expanded this node.
            // The simulation result is this neural network evaluation. Break to backpropagate.
            if (child_value.has_value())
            {
                break;
            }

            // If the node was already expanded, select the best child and continue traversal.
            int move_idx = expansion_strategy_(*currentNode);
            currentNode = currentNode->getChildStates()[move_idx].get();
        }

        // 3. SIMULATION & BACKPROPAGATION
        AverageValue accumulated_value;
        Player winner = Player::NONE;

        // Check if the loop was broken by expansion (child_value has value).
        auto &last_path_item = backprop_stack.back();
        if (last_path_item.second.has_value())
        {
            accumulated_value = last_path_item.second.value();
            // The "winner" in this case isn't a game win, but the perspective for the value.
            // We use the player whose turn it was at the expanded node.
            winner = last_path_item.first->state().getCurrentPlayer();
        }
        // Otherwise, the loop ended because the game is over.
        else if (currentNode->state().isGameOver())
        {
            auto winner_opt = currentNode->state().getWinner();
            if (winner_opt.has_value())
            {
                winner = winner_opt.value();
            }
            accumulated_value.addWinner(winner);
            currentNode->update(winner, accumulated_value);
        }

        // Backpropagate the results up the tree.
        while (!backprop_stack.empty())
        {
            auto path_node = backprop_stack.back().first;
            backprop_stack.pop_back();
            path_node->update(winner, accumulated_value);
        }
    }

}