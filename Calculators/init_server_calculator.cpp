#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

namespace mediapipe {
    class InitServerCalculator : 
        public CalculatorBase {
    public:
        static constexpr char OutputSideTag1[] = "SOCKET";
        static constexpr char OutputSideTag2[] = "START";
        static constexpr char InputSideTag1[] = "PORT";
        static Status GetContract(CalculatorContract *cc) {
            cc->InputSidePackets().Tag(InputSideTag1).Set<int>();
            cc->OutputSidePackets().Tag(OutputSideTag1).Set<int>();
            cc->OutputSidePackets().Tag(OutputSideTag2).Set<int>();
            return OkStatus();
        }

        Status Open(CalculatorContext *cc) override {
            std::cout << "starting server" << std::endl;
            int server_fd, new_socket;
            int PORT = cc->InputSidePackets().Tag(InputSideTag1).Get<int>();//59832
            struct sockaddr_in address;
            int opt = 1;
            int addrlen = sizeof(address);
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
                == 0) {
                return absl::InternalError("Failed to get socket");
            }
            if (setsockopt(server_fd, SOL_SOCKET,
                        SO_REUSEADDR , &opt,
                        sizeof(opt))) {
                return absl::InternalError("Failed to config socket");
            }
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(PORT);
            if (bind(server_fd, (struct sockaddr*)&address,
                    sizeof(address))
                < 0) {
                return absl::InternalError("Failed to bind socket");
            }
            if (listen(server_fd, 10) < 0) {
                return absl::InternalError("Failed to setup listen");
            }
            std::cout << "init complete" << std::endl;
            if ((new_socket
                = accept(server_fd, (struct sockaddr*)&address,
                        (socklen_t*)&addrlen))
                < 0) {
                return absl::InternalError("Failed to connect");
            }
            std::cout << "connected" << std::endl;
            Packet sock_numb = MakePacket<int>(new_socket);
            cc->OutputSidePackets().Tag(OutputSideTag1).Set(sock_numb);

            Packet init = MakePacket<int>(1);
            cc->OutputSidePackets().Tag(OutputSideTag2).Set(init);
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) {
            return OkStatus();
        }
    };
    REGISTER_CALCULATOR(InitServerCalculator);
}
