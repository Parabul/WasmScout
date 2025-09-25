# WasmScout
Nine Pebbles Web Assembly (C++) client

## Install ONNX Runtime

Build shared library
```shell
./build.sh --config=Release --build_shared_lib --minimal_build --disable_ml_ops --disable_exceptions --disable_rtti --skip_tests
```

Build Wasm static library
```shell
/build.sh --build_wasm_static_lib --enable_wasm_simd --disable_wasm_exception_catching --disable_rtti --enable_wasm_threads  --minimal_build --skip_tests --config Release
```

## Build JavaScript/Wasm
```shell
bazel build -c opt -s --config=wasm //lib:hello-main-wasm

node bazel-bin/lib/hello-main-wasm/hello-main.js
```