import onnxruntime as ort

# Path to your original ONNX model
onnx_model_path = "nine_pebbles.onnx"

# Path for the converted ORT model
ort_model_path = "nine_pebbles.ort"

so = ort.SessionOptions()

# This key step tells ONNX Runtime to convert and save the model to the .ort format
so.optimized_model_filepath = ort_model_path

# Set the optimization level
so.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_EXTENDED

# Creating the session will trigger the conversion and optimization
_ = ort.InferenceSession(onnx_model_path, so)

print(f"✅ Model converted successfully to {ort_model_path}")
