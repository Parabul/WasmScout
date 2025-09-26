load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

cc_library(
    name = "model",
    hdrs = ["model.h"],
    visibility = ["//main:__pkg__"],
)

cc_library(
    name = "game",
    hdrs = ["game.h"],
    srcs = ["game.cc"],
    visibility = ["//main:__pkg__"],
)

cc_test(
    name = "game_test",
    srcs = ["game_test.cc"],
    deps = [
        ":game",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "mcts",
    hdrs = ["mcts.h"],
    srcs = ["mcts.cc"],
    deps = [
        ":game",
        "//third_party/onnxruntime",
        ":model",
    ],
    visibility = ["//main:__pkg__"],
)

cc_test(
    name = "mcts_test",
    srcs = ["mcts_test.cc"],
    deps = [
        ":mcts",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "wasm",
    srcs = ["wasm.cc"],
    hdrs = ["wasm.h"],
    deps = [
        ":game",
        ":mcts",
    ],
    visibility = ["//main:__pkg__"],
    
)

cc_binary(
    name = "main",
    srcs = ["wasm_main.cc"],
    deps = [
        ":wasm",
        ":game",
    ],
)

wasm_cc_binary(
    name = "wasm-main",
    cc_target = ":main",
)


cc_test(
    name = "wasm_test",
    srcs = ["wasm_test.cc"],
    deps = [
        ":wasm",
        ":game",
        "@googletest//:gtest_main",
    ],
)