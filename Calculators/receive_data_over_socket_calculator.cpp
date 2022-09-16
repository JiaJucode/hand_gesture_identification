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

namespace mediapipe {
    class ReceiveDataOverSocketCalculator : 
        public CalculatorBase {
    public:
        static constexpr char OutputTag[] = "DATA";
        static constexpr char SidePacketTag[] = "SOCKET";
        static constexpr char InputTag[] = "RECEIVE";

        static Status GetContract(CalculatorContract *cc) {
            const auto& options = 
                cc->Options<::mediapipe::SocketDataCalculatorOptions>();            
            cc->Inputs().Tag(InputTag).Set<int>();
            for (int i = 0; i < options.data_size(); i++) {
                std::string tagname = OutputTag + std::to_string(i);
                switch (options.data(i))
                {
                case mediapipe::SocketDataCalculatorOptions::INT_TYPE: {
                    cc->Outputs().Tag(tagname).Set<int>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::FLOAT_TYPE: {
                    cc->Outputs().Tag(tagname).Set<float>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::STRING_TYPE: {
                    cc->Outputs().Tag(tagname).Set<std::string>();
                    break;                
                }
                case mediapipe::SocketDataCalculatorOptions::VEC_INT_TYPE: {
                    cc->Outputs().Tag(tagname).Set<std::vector<int>>();
                    break;
                }
                case mediapipe::SocketDataCalculatorOptions::VEC_FLOAT_TYPE: {
                    cc->Outputs().Tag(tagname).Set<std::vector<float>>();
                    break;
                }
                }
            }

            cc->InputSidePackets().Tag(SidePacketTag).Set<int>();
            return OkStatus();
        }

        Status Open(CalculatorContext *cc) override {
            sender_sock = 
                cc->InputSidePackets().Tag(SidePacketTag).Get<int>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            sleep(100);
            std::cout << "start receive" << std::endl;
            const auto& options = 
                cc->Options<::mediapipe::SocketDataCalculatorOptions>();
            int ready = cc->Inputs().Tag(InputTag).Value().Get<int>();
            if (ready != 1) {
                std::cout << "not ready: " << ready << std::endl;
                return OkStatus();
            }
            std::cout << "ready" << std::endl;
            sleep(1);
            int receive_size = 1;
            for (int i = 0; i < options.data_size(); i++) {
                std::string tagname = OutputTag + std::to_string(i);
                Packet pOut;

                switch (options.data(i))
                {
                    case mediapipe::SocketDataCalculatorOptions::INT_TYPE: {
                        int data;
                        receive_size = read(sender_sock, &data, sizeof(int));
                        pOut = MakePacket<int>(ntohl(data)).At(cc->InputTimestamp());
                        break;
                    }
                    case mediapipe::SocketDataCalculatorOptions::FLOAT_TYPE: {
                        float float_data;
                        receive_size = read(sender_sock, &float_data, sizeof(float));
                        pOut = MakePacket<float>(float_data).At(cc->InputTimestamp());                    
                        break;
                    }
                    case mediapipe::SocketDataCalculatorOptions::STRING_TYPE: {
                        std::vector<char> string_buffer(MAX_RECEIVE);
                        receive_size = read(sender_sock, &string_buffer, MAX_RECEIVE);
                        std::string str_data;
                        str_data.assign(&(string_buffer[0]), string_buffer.size());
                        pOut = MakePacket<std::string>(str_data).At(cc->InputTimestamp()); 
                        break;           
                    }
                    case mediapipe::SocketDataCalculatorOptions::VEC_INT_TYPE: {
                        std::vector<int> int_buffer(MAX_RECEIVE / sizeof(int));
                        receive_size = read(sender_sock, int_buffer.data(), MAX_RECEIVE);
                        int_buffer.resize(receive_size / sizeof(int));
                        pOut = MakePacket<std::vector<int>>(int_buffer).At(cc->InputTimestamp()); 
                        break;
                    }
                        case mediapipe::SocketDataCalculatorOptions::VEC_FLOAT_TYPE: {
                        std::vector<float> float_buffer(MAX_RECEIVE / sizeof(float));
                        receive_size = read(sender_sock, float_buffer.data(), MAX_RECEIVE);
                        float_buffer.resize(receive_size / sizeof(float));
                        for (auto i: float_buffer) {
                            std::cout << i << ", ";
                        }
                        pOut = MakePacket<std::vector<float>>(float_buffer).At(Timestamp(1)); 
                        break;  
                    }
                }
                if (receive_size <= 0) {
                    std::cout << "not receive" << std::endl;
                    break;
                }
                std::cout << "received" << std::endl;
                cc->Outputs().Tag(tagname).AddPacket(pOut);
                    
            }
            
            return OkStatus();
        }
    private:
        static constexpr int MAX_RECEIVE = 1024;
        int sender_sock;
    };


    REGISTER_CALCULATOR(ReceiveDataOverSocketCalculator);
}