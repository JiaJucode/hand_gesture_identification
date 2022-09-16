#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/examples/subgraph_testing/Calculators/socket_data_calculator.pb.h"
#include <string>
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

template <class T>
int send_vector(int socket_number, std::vector<T> data) {
    return send(socket_number, (const char*)data.data(), data.size() * sizeof(T), 0);
}

namespace mediapipe {
    class SendDataOverSocketCalculator : 
        public CalculatorBase {
    public:
        static constexpr char PacketTag[] = "DATA";
        static constexpr char SidePacketTag[] = "SOCKET";
        static constexpr char OutputTag[] = "SEND";
        static Status GetContract(CalculatorContract *cc) {
            const auto& options = 
                cc->Options<::mediapipe::SocketDataCalculatorOptions>();

            for (int i = 0; i < options.data_size(); i++) {
                std::string tagname = PacketTag + std::to_string(i);
                switch (options.data(i))
                {
                case mediapipe::SocketDataCalculatorOptions::INT_TYPE: {
                    cc->Inputs().Tag(tagname).Set<int>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::FLOAT_TYPE: {
                    cc->Inputs().Tag(tagname).Set<float>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::STRING_TYPE: {
                    cc->Inputs().Tag(tagname).Set<std::string>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::VEC_INT_TYPE: {
                    cc->Inputs().Tag(tagname).Set<std::vector<int>>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::VEC_FLOAT_TYPE: {
                    cc->Inputs().Tag(tagname).Set<std::vector<float>>();
                    break;
                }
                }
            }
            cc->Outputs().Tag(OutputTag).Set<int>();
            cc->InputSidePackets().Tag(SidePacketTag).Set<int>();
            return OkStatus();
        }
        
        Status Open(CalculatorContext *cc) override {
            target_socket_number = 
                cc->InputSidePackets().Tag(SidePacketTag).Get<int>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            const auto& options = 
                cc->Options<::mediapipe::SocketDataCalculatorOptions>();
            for (int i = 0; i < options.data_size(); i++) {
                std::string tagname = PacketTag + std::to_string(i);
                switch (options.data(i))
                {
                case mediapipe::SocketDataCalculatorOptions::INT_TYPE: {
                    auto int_data = htonl(cc->Inputs().Tag(tagname).Value().Get<int>());
                    send(target_socket_number, &int_data, sizeof(int), 0);
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::FLOAT_TYPE: {
                    auto float_data = cc->Inputs().Tag(tagname).Value().Get<float>();
                    send(target_socket_number, &float_data, sizeof(float), 0);
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::STRING_TYPE: {
                    auto str_data = cc->Inputs().Tag(tagname).Value().Get<std::string>();
                    send(target_socket_number, str_data.c_str(), sizeof(str_data.c_str()), 0);
                    break;           
                }
                case mediapipe::SocketDataCalculatorOptions::VEC_INT_TYPE: {
                    auto int_vec_data = cc->Inputs().Tag(tagname).Value().Get<std::vector<int>>();
                    send_vector<int>(target_socket_number, int_vec_data);
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::VEC_FLOAT_TYPE: {
                    auto float_vec_data = cc->Inputs().Tag(tagname).Value().Get<std::vector<float>>();
                    send_vector<float>(target_socket_number, float_vec_data);
                    break;
                }
                }
            }
            Packet pOut = MakePacket<int>(1);
            cc->Outputs().Tag(OutputTag).AddPacket(pOut);
            std::cout << "send complete" << std::endl;
            return OkStatus();
        }
    private:
        int target_socket_number;
    };


    REGISTER_CALCULATOR(SendDataOverSocketCalculator);
}