#include "lib/wasm.h"
#include "lib/game.h"
#include "gtest/gtest.h"

namespace scout
{

    TEST(InferTest, Generic)
    {
        auto root_state = std::make_unique<GameState>();
        //root_state = root_state->move(8)->move(1)->move(7)->move(3)->move(6)->move(3)->move(4)->move(1)->move(8)->move(8);

        EXPECT_EQ(infer(*root_state.get()), 8);
    }

}