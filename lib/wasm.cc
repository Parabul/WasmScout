#include "lib/wasm.h"
#include <string>

#include "lib/game.h"
#include "lib/mcts.h"
#include <iostream>
#include <string>
#include <chrono>
#include <limits>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

using namespace emscripten;
#endif

namespace scout
{

    int infer(const GameState &game_state)
    {
        OnnxEvaluator onnx_evaluator;
        PredictiveUpperConfidenceBound pucb_strategy;
        MonteCarloTreeSearch mcts(std::ref(pucb_strategy), std::ref(onnx_evaluator));

        auto root_state = std::make_unique<GameState>(game_state);

        std::cout << root_state->toString();

        auto root_node = std::make_unique<TreeNode>(std::move(root_state), GameState::NUM_MOVES);

        const int num_expansions = 2000;

        auto start = std::chrono::steady_clock::now();

        for (int i = 0; i < num_expansions; ++i)
        {
            mcts.expand(root_node.get());
        }

        auto end = std::chrono::steady_clock::now();

        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "\nExecution took: " << duration_ms.count() << " milliseconds" << std::endl;

        auto encoded = root_node->encode();
        std::cout << "Encoded: [";
        for (size_t i = 0; i < encoded.size(); ++i)
        {
            std::cout << encoded[i] << ",";
        }
        std::cout << "] ";

        int best_move = 0;
        float best_value = 0;
        for (size_t i = 1; i < encoded.size(); ++i)
        {
            if (best_value < encoded[i])
            {
                best_move = i;
                best_value = encoded[i];
            }
        }

        return best_move - 1;
    }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(my_module)
{
    function("infer", &scout::infer);
}
#endif