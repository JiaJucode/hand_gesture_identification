load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_simple_subgraph",
)

cc_binary(
    name = "hand_recog",
    srcs = ["main.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/examples/subgraph_testing/Calculators:cout_calculator",
        "//mediapipe/graphs/hand_tracking:desktop_tflite_calculators",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)

cc_binary(
    name = "test",
    srcs = ["test.cpp"],
    deps = [
        "//mediapipe/examples/subgraph_testing/graphs:MLPredictionSubgraph",
        "//mediapipe/examples/subgraph_testing/Calculators:hand_info_extraction_calculator",
        "//mediapipe/calculators/core:constant_side_packet_calculator",
        "//mediapipe/graphs/hand_tracking:desktop_tflite_calculators",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)

cc_binary(
    name = "socket_test",
    srcs = ["socket_test.cpp"],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:status",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        ":MLPredictionSubgraph",
    ],
)
