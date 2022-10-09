load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_simple_subgraph",
)

mediapipe_simple_subgraph(
    name = "MLPredictionSubgraph",
    graph = "MLPredictionSubgraph.pbtxt",
    register_as = "MLPredictionSubgraph",
    deps = [
        "//mediapipe/examples/subgraph_testing/Calculators:send_data_over_socket_calculator",
        "//mediapipe/examples/subgraph_testing/Calculators:receive_data_over_socket_calculator",
        "//mediapipe/examples/subgraph_testing/Calculators:packet_time_delay_calculator",
        "//mediapipe/examples/subgraph_testing/Calculators:init_server_calculator",

        "//mediapipe/framework/stream_handler:early_close_input_stream_handler",

        # mediapipe inference calculator doesnt support SELECT_TF_OPS
        #"//mediapipe/calculators/tflite:tflite_model_calculator",
        #"//mediapipe/calculators/util:local_file_contents_calculator", 
        #"//mediapipe/calculators/tensor:inference_calculator",        
        #"//mediapipe/examples/subgraph_testing/Calculators:mediapipe_tensor_to_vector_float_calculator",
        #"//mediapipe/examples/subgraph_testing/Calculators:vector_float_to_mediapipe_tensor_calculator",
    ],
    visibility = ["//visibility:public"],
)
