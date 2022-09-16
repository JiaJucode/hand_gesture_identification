#include <vector>
#include <iostream>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"

constexpr std::pair<int, int> HAND_PALM_CONNECTIONS[6] = 
    {std::pair(0, 1), std::pair(0, 5), std::pair(9, 13), std::pair(13, 17), std::pair(5, 9), std::pair(0, 17)};
constexpr int HAND_PALM_LANDMARKS[7] = {0, 1, 5, 9, 13, 17};
constexpr std::pair<int, int> HAND_THUMB_CONNECTIONS[3] = {std::pair(1, 2), std::pair(2, 3), std::pair(3, 4)};
constexpr int HAND_THUMB_LANDMARKS[4] = {1, 2, 3, 4};

std::vector<float> getLineInfos(const std::pair<int, int> *connections, int numbOfConnections, float (*pts)[2]) {
    std::vector<float> returnValue;
    float line_vecs[numbOfConnections][2];
    for (int i = 0; i < numbOfConnections; i++) {
        int index1 = (connections + i)->first;
        int index2 = (connections + i)->second;
        line_vecs[i][0] = pts[index1][0] - pts[index2][0];
        line_vecs[i][1] = pts[index1][1] - pts[index2][1];
    }
    //distance
    for (auto line_vec : line_vecs) {
        returnValue.push_back(sqrt(pow(line_vec[0], 2) + pow(line_vec[1], 2)));
    }
    //angle
    for (int i = 0; i < numbOfConnections - 1; i++) {
        float m1 = line_vecs[i][0] / line_vecs[i][1];
        float m2 = line_vecs[i + 1][0] / line_vecs[i + 1][1];
        float angle = atan((m1 - m2)/(1 + m1 * m2));
        if (abs(angle) > M_PI) {
            angle = 2 * M_PI - abs(angle);
        }
        returnValue.push_back(abs(angle));
    }
    return returnValue;
}

void normalizeLandmarks(mediapipe::NormalizedLandmarkList landmarks, mediapipe::NormalizedRect hand_rect, float (*result)[2]) {
    float top_x = hand_rect.x_center() - hand_rect.width() / 2 * cos(hand_rect.rotation()) 
        + hand_rect.height() / 2 * sin(hand_rect.rotation());
    float top_y = hand_rect.y_center() - hand_rect.width() / 2 * sin(hand_rect.rotation())
        - hand_rect.height() / 2 * cos(hand_rect.rotation());

    for (int i = 0; i < landmarks.landmark_size(); i++) {
        //translate x and y coords
        float shift_x = (landmarks.landmark(i).x() - top_x);
        float shift_y = (landmarks.landmark(i).y() - top_y);
        //rotate and normalize
        result[i][0] = (cos(hand_rect.rotation()) * shift_x + sin(hand_rect.rotation()) * shift_y) / hand_rect.width();
        result[i][1] = (cos(hand_rect.rotation()) * shift_y - sin(hand_rect.rotation()) * shift_x) / hand_rect.height();
    }
}

namespace mediapipe {
    class HandInfoExtractionCalculator : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            cc->Inputs().Tag("LANDMARKS").SetAny();
            cc->Inputs().Tag("HAND_ROIS_FROM_PALM_DETECTIONS").SetAny();
            cc->Outputs().Tag("THUMB").Set<std::vector<float>>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            Packet landmarks = cc->Inputs().Tag("LANDMARKS").Value();
            Packet rois_palm = cc->Inputs().Tag("HAND_ROIS_FROM_PALM_DETECTIONS").Value();
            if (!landmarks.IsEmpty() && !rois_palm.IsEmpty()) {
                std::vector<mediapipe::NormalizedLandmarkList> landmark_lists = 
                    landmarks.Get<std::vector<mediapipe::NormalizedLandmarkList>>();
                std::vector<mediapipe::NormalizedRect> hand_rects = 
                    rois_palm.Get<std::vector<mediapipe::NormalizedRect>>();
                float landmark_array[landmark_lists.at(0).landmark_size()][2];
                normalizeLandmarks(landmark_lists.at(0), hand_rects.at(0), landmark_array);
                Packet pOut = 
                    MakePacket<std::vector<float>>(getLineInfos(HAND_PALM_CONNECTIONS, 6, landmark_array))
                        .At(cc->InputTimestamp());
                cc->Outputs().Tag("THUMB").AddPacket(pOut);
            }
            return OkStatus();
        }
    };

    REGISTER_CALCULATOR(HandInfoExtractionCalculator);
}