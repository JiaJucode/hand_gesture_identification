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
std::vector<int> HAND_THUMB_LANDMARKS = {1, 2, 3, 4};
constexpr std::pair<int, int> HAND_INDEX_CONNECTIONS[3] = {std::pair(5, 6), std::pair(6, 7), std::pair(7, 8)};
std::vector<int> HAND_INDEX_LANDMARKS[4] = {5, 6, 7, 8};
constexpr std::pair<int, int> HAND_MIDDLE_CONNECTIONS[3] = {std::pair(9, 10), std::pair(10, 11), std::pair(11, 12)};
std::vector<int> HAND_MIDDLE_LANDMARKS[4] = {9, 10, 11, 12};
std::pair<int, int> HAND_RING_CONNECTIONS[3] = {std::pair(13, 14), std::pair(14, 15), std::pair(15, 16)};
std::vector<int> HAND_RING_LANDMARKS[4] = {13, 14, 15, 16};
std::pair<int, int> HAND_LITTLE_CONNECTIONS[3] = {std::pair(17, 18), std::pair(18, 19), std::pair(19, 20)};
std::vector<int> HAND_LITTLE_LANDMARKS[4] = {17, 18, 19, 20}; 

std::vector<float> getZCoords(std::vector<int> thumb_landmarks, mediapipe::NormalizedLandmarkList landmarks) {
    std::vector<float> returnValue;
    for (int i = 0; i < landmarks.landmark_size(); i++) {
        if (std::find(std::begin(HAND_PALM_LANDMARKS), std::end(HAND_PALM_LANDMARKS), i) 
                    != std::end(HAND_PALM_LANDMARKS) ||
                std::find(std::begin(thumb_landmarks), std::end(thumb_landmarks), i) 
                    != std::end(thumb_landmarks)) {
                returnValue.push_back(landmarks.landmark(i).z());
        }
    }
    return returnValue;
}

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

std::vector<float> fingerOutput(std::vector<int> finger_landmarks,mediapipe::NormalizedLandmarkList landmarks, 
    const std::pair<int, int> *connections, int numbOfConnections, float (*pts)[2], std::vector<float> *palm_infos) {
    std::vector<float> output = getZCoords(finger_landmarks, landmarks);
    std::vector<float> infos = getLineInfos(connections, numbOfConnections, pts);
    output.insert(output.end(), infos.begin(), infos.end());
    output.insert(output.end(), palm_infos->begin(), palm_infos->end());
    return output;
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

                std::vector<float> palm_infos = getLineInfos(HAND_PALM_CONNECTIONS, 6, landmark_array);
                Packet thumbOut = 
                    MakePacket<std::vector<float>>(fingerOutput(HAND_THUMB_LANDMARKS, landmark_lists.at(0), HAND_THUMB_CONNECTIONS, 3, landmark_array, &palm_infos))
                        .At(cc->InputTimestamp());
                cc->Outputs().Tag("THUMB").AddPacket(thumbOut);
                /*
                Packet indexOut = 
                    MakePacket<std::vector<float>>(fingerOutput(HAND_INDEX_LANDMARKS, landmark_lists.at(0), HAND_INDEX_CONNECTIONS, 3, landmark_array, &palm_infos))
                        .At(cc->InputTimestamp());
                cc->Outputs().Tag("INDEX").AddPacket(indexOut);
                Packet middleOut = 
                    MakePacket<std::vector<float>>(fingerOutput(HAND_MIDDLE_LANDMARKS, landmark_lists.at(0), HAND_MIDDLE_CONNECTIONS, 3, landmark_array, &palm_infos))
                        .At(cc->InputTimestamp());
                cc->Outputs().Tag("MIDDLE").AddPacket(middleOut);
                Packet ringOut = 
                    MakePacket<std::vector<float>>(fingerOutput(HAND_RING_LANDMARKS, landmark_lists.at(0), HAND_RING_CONNECTIONS, 3, landmark_array, &palm_infos))
                        .At(cc->InputTimestamp());
                cc->Outputs().Tag("RING").AddPacket(ringOut);
                Packet littleOut = 
                    MakePacket<std::vector<float>>(fingerOutput(HAND_LITTLE_LANDMARKS, landmark_lists.at(0), HAND_LITTLE_CONNECTIONS, 3, landmark_array, &palm_infos))
                        .At(cc->InputTimestamp());
                cc->Outputs().Tag("LITTLE").AddPacket(littleOut);
                */
            }
            else {
                return absl::InternalError("Hand not detected");
            }
            return OkStatus();
        }
    };

    REGISTER_CALCULATOR(HandInfoExtractionCalculator);
}