#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <utility>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/classification.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"

constexpr std::pair<int, int> HAND_PALM_CONNECTIONS[6] = {std::pair(0, 1), std::pair(0, 5), std::pair(9, 13), std::pair(13, 17), std::pair(5, 9), std::pair(0, 17)};
constexpr int HAND_PALM_LANDMARKS[7] = {0, 1, 5, 9, 13, 17};
constexpr std::pair<int, int> HAND_THUMB_CONNECTIONS[3] = {std::pair(1, 2), std::pair(2, 3), std::pair(3, 4)};
std::vector<int> HAND_THUMB_LANDMARKS = {1, 2, 3, 4};
constexpr std::pair<int, int> HAND_INDEX_FINGER_CONNECTIONS[3] = {std::pair(5, 6), std::pair(6, 7), std::pair(7, 8)};
std::vector<int> HAND_INDEX_FINGER_LANDMARKS = {5, 6, 7, 8};
constexpr std::pair<int, int> HAND_MIDDLE_FINGER_CONNECTIONS[3] = {std::pair(9, 10), std::pair(10, 11), std::pair(11, 12)};
std::vector<int> HAND_MIDDLE_FINGER_LANDMARKS = {9, 10, 11, 12};
std::pair<int, int> HAND_RING_FINGER_CONNECTIONS[3] = {std::pair(13, 14), std::pair(14, 15), std::pair(15, 16)};
std::vector<int> HAND_RING_FINGER_LANDMARKS = {13, 14, 15, 16};
std::pair<int, int> HAND_PINKY_FINGER_CONNECTIONS[3] = {std::pair(17, 18), std::pair(18, 19), std::pair(19, 20)};
std::vector<int> HAND_PINKY_FINGER_LANDMARKS = {17, 18, 19, 20}; 
std::string decodeGestureType(int gesture_type) {
    if (gesture_type == 32) {
        return (std::to_string(0) + ", " + std::to_string(0));
    }
    std::string returnString = "";
    if (gesture_type > 1) {
        returnString += std::to_string(1);
    } else {
        returnString += std::to_string(0);
    }
    returnString += ", " + std::to_string(gesture_type % 2);
    return returnString;
}

std::string getLineInfos(const std::pair<int, int> *connections, int numbOfConnections, float (*pts)[2]) {
    std::string returnString;
    float line_vecs[numbOfConnections][2];
    for (int i = 0; i < numbOfConnections; i++) {
        int index1 = (connections + i)->first;
        int index2 = (connections + i)->second;
        line_vecs[i][0] = pts[index1][0] - pts[index2][0];
        line_vecs[i][1] = pts[index1][1] - pts[index2][1];
    }
    //distance
    for (auto line_vec : line_vecs) {
        returnString += ", " + std::to_string(sqrt(pow(line_vec[0], 2) + pow(line_vec[1], 2)));
    }
    //angle
    for (int i = 0; i < numbOfConnections - 1; i++) {
        float m1 = line_vecs[i][0] / line_vecs[i][1];
        float m2 = line_vecs[i + 1][0] / line_vecs[i + 1][1];
        float angle = atan((m1 - m2)/(1 + m1 * m2));
        if (abs(angle) > M_PI) {
            angle = 2 * M_PI - abs(angle);
        }
        returnString += ", " + std::to_string(abs(angle));
    }
    return returnString;
}

void writeToCSV(mediapipe::NormalizedLandmarkList inputs, mediapipe::NormalizedRect handRect, int gesture_type, const std::pair<int, int> *connections, std::vector<int> finger_landmarks) {
    std::fstream fout;

    fout.open("//Users/negativetraffic/Documents/gesture_data/index_data/traindata1.csv", std::ios::out | std::ios::app);

    float top_x = handRect.x_center() - handRect.width() / 2 * cos(handRect.rotation()) 
        + handRect.height() / 2 * sin(handRect.rotation());
    float top_y = handRect.y_center() - handRect.width() / 2 * sin(handRect.rotation())
        - handRect.height() / 2 * cos(handRect.rotation());
    

    fout << decodeGestureType(gesture_type);
    float pts[inputs.landmark_size()][2];
    for (int i = 0; i < inputs.landmark_size(); i++) {
        //translate x and y coords
        float shift_x = (inputs.landmark(i).x() - top_x);
        float shift_y = (inputs.landmark(i).y() - top_y);
        //rotate and normalize
        pts[i][0] = (cos(handRect.rotation()) * shift_x + sin(handRect.rotation()) * shift_y) / handRect.width();
        pts[i][1] = (cos(handRect.rotation()) * shift_y - sin(handRect.rotation()) * shift_x) / handRect.height();
        if (std::find(std::begin(HAND_PALM_LANDMARKS), std::end(HAND_PALM_LANDMARKS), i) 
                != std::end(HAND_PALM_LANDMARKS) ||
            std::find(std::begin(finger_landmarks), std::end(finger_landmarks), i) 
                != std::end(finger_landmarks)) {
            fout << ", " << inputs.landmark(i).z();
        }
    }
    fout << getLineInfos(connections, 3, pts);
    fout << getLineInfos(HAND_PALM_CONNECTIONS, 6, pts);
    fout << "\n";

}


namespace mediapipe {
    class CoutCalculator : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            cc->Inputs().Tag("LANDMARKS").SetAny();
            cc->Inputs().Tag("HANDEDNESS").SetAny();
            cc->Inputs().Tag("PALM_DETECTIONS").SetAny();
            cc->Inputs().Tag("HAND_ROIS_FROM_LANDMARKS").SetAny();
            cc->Inputs().Tag("HAND_ROIS_FROM_PALM_DETECTIONS").SetAny();
            cc->Inputs().Tag("WRITE").Set<int>();
            return OkStatus();
        }

        Status Process(CalculatorContext *cc) override {
            Packet landmarks = cc->Inputs().Tag("LANDMARKS").Value();
            Packet handedness = cc->Inputs().Tag("HANDEDNESS").Value();
            Packet palm = cc->Inputs().Tag("PALM_DETECTIONS").Value();
            Packet rois_landmarks = cc->Inputs().Tag("HAND_ROIS_FROM_LANDMARKS").Value();
            Packet rois_palm = cc->Inputs().Tag("HAND_ROIS_FROM_PALM_DETECTIONS").Value();
            Packet write = cc->Inputs().Tag("WRITE").Value();

            if (!landmarks.IsEmpty() && !rois_palm.IsEmpty()) {
                std::vector<mediapipe::NormalizedLandmarkList> landmark_list = landmarks.Get<std::vector<mediapipe::NormalizedLandmarkList, std::__1::allocator<mediapipe::NormalizedLandmarkList> >>();

                std::vector<mediapipe::NormalizedRect> hand_rects = rois_palm.Get<std::vector<mediapipe::NormalizedRect>>();
 
                int gesture_type = write.Get<int>();

                if (gesture_type != 0) {
                    writeToCSV(landmark_list.at(0), hand_rects.at(0), gesture_type, HAND_INDEX_FINGER_CONNECTIONS, HAND_INDEX_FINGER_LANDMARKS);
                }
                
            }
            return OkStatus();
        }
    };

    REGISTER_CALCULATOR(CoutCalculator);
}