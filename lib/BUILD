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
        "//third_party/onnxruntime:onnxruntime",
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
    name = "hello",
    srcs = ["hello.cc"],
    hdrs = ["hello.h"],
    deps = [
        ":game",
        ":mcts",
    ],
    visibility = ["//main:__pkg__"],
    
)

cc_binary(
    name = "hello-main",
    srcs = ["hello_main.cc"],
    deps = [
        ":hello",
    ],
)

wasm_cc_binary(
    name = "hello-main-wasm",
    cc_target = ":hello-main",
)


cc_test(
    name = "hello_test",
    srcs = ["hello_test.cc"],
    deps = [
        ":hello",
        "@googletest//:gtest_main",
    ],
)