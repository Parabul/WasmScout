# ONNX Runtime static/shared libraries for WASM/Linux builds.
cc_import(
    name = "onnxruntime_webassembly_lib",
    static_library = "lib/libonnxruntime_webassembly.a",  # Path to the static library file
)

cc_import(
    name = "onnxruntime_lib",
    shared_library = "lib/libonnxruntime.so.1",
    visibility = ["//visibility:public"],
)


cc_library(
    name = "onnxruntime_webassembly",
    hdrs = glob(["include/**/*.h"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:private"],
    deps = [":onnxruntime_webassembly_lib"],
)

cc_library(
    name = "onnxruntime",
    hdrs = glob(["include/**/*.h"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = 
    select({
        "//:wasm_build": [":onnxruntime_webassembly_lib"],
        "//conditions:default": [":onnxruntime_lib"]
    }),
)