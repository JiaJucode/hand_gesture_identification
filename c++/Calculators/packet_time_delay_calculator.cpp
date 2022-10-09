#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"
#include <iostream>

namespace mediapipe {
    class PacketTimeDelayCalculator : 
        public CalculatorBase {
    public: 
        static Status GetContract(CalculatorContract *cc) {
            RET_CHECK_EQ(cc->Inputs().NumEntries(), 1) << "only one input stream is supported";
            RET_CHECK_EQ(cc->Outputs().NumEntries(), 1) << "only one output stream is supported";
            cc->Inputs().Index(0).Set<int>();
            cc->Outputs().Index(0).Set<int>();
            cc->InputSidePackets().Index(0).Set<int>();
            return OkStatus();
        }

        Status Open(CalculatorContext *cc) override {
            //const auto& options = 
            //    cc->Options<::mediapipe::SocketDataCalculatorOptions>();
            int initial_value = cc->InputSidePackets().Index(0).Get<int>();
            Packet init = MakePacket<int>(initial_value).At(Timestamp(0));
            cc->Outputs().Index(0).AddPacket(init);
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) {
            int value = cc->Inputs().Index(0).Value().Get<int>();
            Packet next_packet = MakePacket<int>(value).At(cc->InputTimestamp().NextAllowedInStream());
            cc->Outputs().Index(0).AddPacket(next_packet);
            return OkStatus();
        }
    };
    REGISTER_CALCULATOR(PacketTimeDelayCalculator);
}
