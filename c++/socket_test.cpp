#include <string>
#include <iostream>
#include <vector>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

constexpr char FILE_PATH[] = "mediapipe/examples/c++/graphs/MLPredictionSubgraph.pbtxt";
constexpr char INPUT_STREAM[] = "input_vec";
constexpr char OUTPUT_STREAM[] = "result";

absl::Status run() {
    std::string graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
        FILE_PATH, &graph_config_contents));

    mediapipe::CalculatorGraphConfig config = 
        mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
            graph_config_contents);

    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
        graph.AddOutputStreamPoller(OUTPUT_STREAM));

//start graph
    MP_RETURN_IF_ERROR(graph.StartRun({}));

//send data

    std::vector<float> send_data{2.74559e-07, -0.013148, -0.0155419, -0.0179827, -0.0210028, -0.000436373, -0.00347011, -0.00940575, -0.0171704, 0.161262, 0.153599, 0.101334, 0.916365, 1.005153, 0.151101, 0.402794, 0.087270, 0.088359, 0.104345, 0.416706, 0.851858, 1.238050, 0.342409, 0.773874, 0.812670};
    size_t timestamp = 0;
    auto data_vec = absl::make_unique<std::vector<float>>(send_data);
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(data_vec.release())
            .At(mediapipe::Timestamp(timestamp))));

    std::vector<float> send_data2{2.75179e-07, -0.0130221, -0.0150161, -0.0171573, -0.0199874, 0.000784876, -0.00229499, -0.0083347, -0.0161887, 0.162293, 0.146973, 0.096689, 0.916115, 1.147246, 0.153705, 0.399450, 0.086766, 0.088929, 0.101222, 0.404094, 0.813328, 1.200390, 0.323390, 0.753837, 0.842792};
    auto data_vec2 = absl::make_unique<std::vector<float>>(send_data2);
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(data_vec2.release())
            .At(mediapipe::Timestamp(timestamp + 1))));

    std::cout << "data send" << std::endl;
//receive data
    mediapipe::Packet packet;
    for (int i = 0; i != 2; i++) {
        poller.Next(&packet);
        auto& result = packet.Get<std::vector<float>>();

        for (int i = 0; i != result.size(); i++) {
            std::cout << result.at(i) << ", ";
        }
        std::cout << "l" << std::endl;

    }
    MP_RETURN_IF_ERROR(graph.CloseInputStream(INPUT_STREAM));
    return graph.WaitUntilDone();
}

int main() {
    absl::Status status = run();
    std::cout << "status =" << status << std::endl;
    std::cout << "status.ok() = " << status.ok() << std::endl;

}