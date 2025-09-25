import onnxruntime as ort

# Path to your original ONNX model
onnx_model_path = "nine_pebbles.onnx"

# Path for the converted ORT model
ort_model_path = "nine_pebbles.ort"

so = ort.SessionOptions()

so.optimized_model_filepath = ort_model_path
so.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_EXTENDED

# Creating the session will trigger the conversion and optimization
_ = ort.InferenceSession(onnx_model_path, so)

print(f"Model converted successfully to {ort_model_path}")
