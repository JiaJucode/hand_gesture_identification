#include <vector>
#include <memory>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/tensor.h"
#include "mediapipe/framework/port/ret_check.h"


namespace mediapipe {
    class MediapipeTensorToVectorFloatCalculator :
        public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            RET_CHECK_EQ(cc->Inputs().NumEntries(), 1) << "only one input stream is supported";
            RET_CHECK_EQ(cc->Outputs().NumEntries(), 1) << "only one output stream is supported";
            cc->Inputs().Index(0).Set<std::vector<Tensor>>();
            cc->Outputs().Index(0).Set<std::vector<float>>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            const Tensor& tensor = 
                cc->Inputs().Index(0).Value().Get<std::vector<Tensor>>().at(0);
            auto raw_data = tensor.GetCpuReadView().buffer<float>();

            std::vector<float> float_vec(raw_data, raw_data + tensor.shape().num_elements());

            Packet result = MakePacket<std::vector<float>>(float_vec).At(cc->InputTimestamp());
            cc->Outputs().Index(0).AddPacket(result);
            return OkStatus();
        }
    };

    REGISTER_CALCULATOR(MediapipeTensorToVectorFloatCalculator);
}