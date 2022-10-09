#include <vector>
#include <memory>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/tensor.h"
#include "mediapipe/framework/port/ret_check.h"


namespace mediapipe {
    class VectorFloatToMediapipeTensorCalculator :
        public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            RET_CHECK_EQ(cc->Inputs().NumEntries(), 1) << "only one input stream is supported";
            RET_CHECK_EQ(cc->Outputs().NumEntries(), 1) << "only one output stream is supported";
            cc->Inputs().Index(0).Set<std::vector<float>>();
            cc->Outputs().Index(0).Set<std::vector<Tensor>>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            std::vector<float> float_vec = 
                cc->Inputs().Index(0).Value().Get<std::vector<float>>();

            const int vec_size = float_vec.size();
            auto tensor_shape = Tensor::Shape{1, vec_size};
            Tensor tensor(Tensor::ElementType::kFloat32, tensor_shape);
            auto* buffer = tensor.GetCpuWriteView().buffer<float>();

            for (int i = 0; i < float_vec.size(); i++) {
                buffer[i] = float_vec.at(i);
            }

            auto tensor_vec = std::vector<Tensor>();
            tensor_vec.push_back(std::move(tensor));
            Packet result = MakePacket<std::vector<Tensor>>(std::move(tensor_vec)).At(cc->InputTimestamp());
            cc->Outputs().Index(0).AddPacket(result);
            return OkStatus();
        }
    };

    REGISTER_CALCULATOR(VectorFloatToMediapipeTensorCalculator);
}