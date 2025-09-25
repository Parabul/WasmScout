#include "lib/hello.h"
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

    std::string get_greet(const std::string &who)
    {
        std::stringstream ss;
        ss << "Hello";
        ss << who;
        ss << static_cast<int>(Player::ONE);
        return ss.str();
    }

    // Use EMSCRIPTEN_KEEPALIVE to export the function.
    // EMSCRIPTEN_KEEPALIVE
    int add(int a, int b)
    {
        return a + b;
    }

    int infer()
    {
        scout::OnnxEvaluator onnx_evaluator;

        // 🧭 Initialize the expansion strategy.
        scout::PredictiveUpperConfidenceBound pucb_strategy;

        // 🌲 Create the main MCTS orchestrator.
        scout::MonteCarloTreeSearch mcts(std::ref(pucb_strategy), std::ref(onnx_evaluator));

        // 🌳 Create the root node for a new game.
        auto root_state = std::make_unique<scout::GameState>();
        // root_state = root_state->move(8)->move(1)->move(7)->move(3)->move(6)->move(3)->move(4)->move(1)->move(8)->move(8);

        // 1. 98 (10), 22 (10)
        // 2. 87 (22), 46 (20)
        // 3. 76 (36), 45
        // 4. 55 (52), 25
        // 5. 93 (68), 91!
        // 6. 91 (84)

        std::cout << root_state->toString();

        auto root_node = std::make_unique<scout::TreeNode>(std::move(root_state), scout::GameState::NUM_MOVES);

        const int num_expansions = 2000;

        // --- 2. ACT: Run the MCTS expansion loop ---

        auto start = std::chrono::steady_clock::now();
        // Perform the search for a fixed number of iterations.
        for (int i = 0; i < num_expansions; ++i)
        {
            mcts.expand(root_node.get());
        }
        auto end = std::chrono::steady_clock::now();

        // 3. Calculate the duration
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // 4. Print the result
        std::cout << "\nExecution took: " << duration_ms.count() << " milliseconds" << std::endl;
        std::cout << "Execution took: " << duration_us.count() << " microseconds" << std::endl;

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

        return best_move;
    }

    // OrtThreadingOptions
    // Ort::ThreadingOptions tp_options;
    // tp_options.SetGlobalInterOpNumThreads(1);
    // tp_options.SetGlobalIntraOpNumThreads(1);

    // Ort::Env ort_env(tp_options, OrtLoggingLevel::ORT_LOGGING_LEVEL_INFO);

    // Ort::Session session{ort_env, nine_pebbles_ort, nine_pebbles_ort_len, Ort::SessionOptions{nullptr}};
    // auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    // std::array<float, 47> input_data{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.0, 0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.11111111, 0.012345679, 0.12345679, 0.12345679, 0.0, 0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.12345679, 0.0, 0.12345679, 0.024691358, 0.0};
    // std::array<int64_t, 2> input_shape{1, 47};
    // Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info,
    //                                                           input_data.data(), input_data.size(),
    //                                                           input_shape.data(), input_shape.size());

    // std::array<float, 6> output_data{};
    // std::array<int64_t, 2> output_shape{1, 1};
    // Ort::Value output_tensor = Ort::Value::CreateTensor<float>(memory_info,
    //                                                            output_data.data(), output_data.size(),
    //                                                            output_shape.data(), output_shape.size());

    // const char *input_names[] = {"input_1"};
    // const char *output_names[] = {"value_output"};

    // auto start = std::chrono::steady_clock::now();
    // for (int i = 0; i < 100000; ++i)
    // {
    //     session.Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, &output_tensor, 1);
    // }
    // auto end = std::chrono::steady_clock::now();

    // // 3. Calculate the duration
    // auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // // 4. Print the result
    // std::cout << "\nExecution took: " << duration_ms.count() << " milliseconds" << std::endl;
    // std::cout << "Execution took: " << duration_us.count() << " microseconds" << std::endl;

    // auto type_info = output_tensor.GetTensorTypeAndShapeInfo();
    // auto total_len = type_info.GetElementCount();

    // float *result = output_tensor.GetTensorMutableData<float>();
    // int val = -1;
    // for (size_t i = 0; i != total_len; ++i)
    // {
    //     std::cout << i << ": " << result[i] << std::endl;
    //     val = (int)(result[i] * 10000);
    // }

    // return val;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(my_module)
{
    function("infer", &scout::infer);
}
#endif