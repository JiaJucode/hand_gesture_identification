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

constexpr char FILE_PATH[] = "mediapipe/examples/subgraph_testing/MLPredictionSubgraph.pbtxt";
constexpr char INPUT_STREAM[] = "input_vec";
constexpr char OUTPUT_STREAM[] = "result";
constexpr char SIDE_INPUT_PACKET[] = "send_sock";

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


//start server
    int server_fd, new_socket;
    const int PORT = 59832;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        std::cout << "socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR , &opt,
                   sizeof(opt))) {
        std::cout <<"setsockopt" << std::endl;
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        std::cout << "listen" << std::endl;
        exit(EXIT_FAILURE);
    }
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))
        < 0) {
        std::cout << "accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }
//start graph
    std::cout << "new socket: " << new_socket << std::endl;
    mediapipe::Packet side = mediapipe::MakePacket<int>(new_socket);
    MP_RETURN_IF_ERROR(graph.StartRun({{SIDE_INPUT_PACKET, side}}));

//send data
/*
    std::vector<float> send_data;
    for (int i = 0; i != 25; i++) {
        send_data.push_back(i / 10);
    }

    size_t timestamp = 1;
    auto data_vec = absl::make_unique<std::vector<float>>(send_data);
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(data_vec.release())
            .At(mediapipe::Timestamp(timestamp))));

    std::vector<float> send_data2;
    for (int i = 0; i != 25; i++) {
        send_data2.push_back(i / 10);
    }
    auto data_vec2 = absl::make_unique<std::vector<float>>(send_data2);
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(data_vec2.release())
            .At(mediapipe::Timestamp(timestamp + 1)))); 
            */
    size_t timestamp = 1;
    int send_data = 1;
    auto data_ptr = absl::make_unique<int>(send_data);
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(data_ptr.release())
            .At(mediapipe::Timestamp(timestamp + 1)))); 
    std::cout << "data send" << std::endl;
//receive data
    while (true) {
        mediapipe::Packet packet;
        if (!poller.Next(&packet)) {
            std::cout << "no more packets" << std::endl;
                break;
        }

        auto& result = packet.Get<std::vector<float>>();

        for (int i = 0; i != result.size(); i++) {
            std::cout << result.at(i) << ", ";
        }
        std::cout << "l" << std::endl;
    }
    
    MP_RETURN_IF_ERROR(graph.CloseInputStream(OUTPUT_STREAM));
    return graph.WaitUntilDone();
}

int main() {
    absl::Status status = run();
    std::cout << "status =" << status << std::endl;
    std::cout << "status.ok() = " << status.ok() << std::endl;

}