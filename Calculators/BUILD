load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")

cc_library(
    name = "cout_calculator",
    srcs = ["cout_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/framework/formats:classification_cc_proto",
    ],
    alwayslink = 1,
)

cc_library(
    name = "hand_info_extraction_calculator",
    srcs = ["hand_info_extraction_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/formats:landmark_cc_proto",
    ],
    alwayslink = 1,
)

cc_library(
    name = "mediapipe_tensor_to_vector_float_calculator",
    srcs = ["mediapipe_tensor_to_vector_float_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:tensor",
        "//mediapipe/framework/port:ret_check",
    ],
    alwayslink = 1,
)

cc_library(
    name = "vector_float_to_mediapipe_tensor_calculator",
    srcs = ["vector_float_to_mediapipe_tensor_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:tensor",
        "//mediapipe/framework/port:ret_check",
    ],
    alwayslink = 1,
)

mediapipe_proto_library(
    name = "socket_data_calculator_proto",
    srcs = ["socket_data_calculator.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
    ],
)

cc_library(
    name = "send_data_over_socket_calculator",
    srcs = ["send_data_over_socket_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        ":socket_data_calculator_cc_proto",
    ],
    alwayslink = 1,
)

cc_library(
    name = "receive_data_over_socket_calculator",
    srcs = ["receive_data_over_socket_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        ":socket_data_calculator_cc_proto",
    ],
    alwayslink = 1,
)

cc_library(
    name = "test_calculator",
    srcs = ["test_calculator.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
