#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include <string>
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mediapipe {
    class TestCalculator : 
        public CalculatorBase {
    public:
        static constexpr char OutputTag[] = "DATA0";
        static constexpr char SidePacketTag[] = "SOCKET";
        static constexpr char InputTag[] = "RECEIVE";

        static Status GetContract(CalculatorContract *cc) {         
            cc->Inputs().Tag(InputTag).Set<int>();
            cc->Outputs().Tag(OutputTag).Set<int>();
            cc->InputSidePackets().Tag(SidePacketTag).Set<int>();
            return OkStatus();
        }

        Status Open(CalculatorContext *cc) override {
            sender_sock = 
                cc->InputSidePackets().Tag(SidePacketTag).Get<int>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            std::cout << "start receive" << std::endl;
            Packet p = cc->Inputs().Tag(InputTag).Value();

            int ready = p.Get<int>();
            if (ready != 1) {
                std::cout << "not ready: " << ready << std::endl;
                return OkStatus();
            }
            std::cout << "ready" << std::endl;
            std::vector<float> send_data;
            for (int i = 0; i != 25; i++) {
                send_data.push_back(i / 10);
            }
            Packet pOut = MakePacket<std::vector<float>>(send_data);
            cc->Outputs().Tag(OutputTag).AddPacket(pOut);
            return OkStatus();
        }
    private:
        static constexpr int MAX_RECEIVE = 1024;
        int sender_sock;
    };


    REGISTER_CALCULATOR(TestCalculator);
}