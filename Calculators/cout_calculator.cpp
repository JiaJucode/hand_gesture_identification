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
constexpr int HAND_THUMB_LANDMARKS[4] = {1, 2, 3, 4};
constexpr std::pair<int, int> HAND_INDEX_FINGER_CONNECTIONS[3] = {std::pair(5, 6), std::pair(6, 7), std::pair(7, 8)};
constexpr int HAND_INDEX_FINGER_LANDMARKS[4] = {5, 6, 7, 8};
constexpr std::pair<int, int> HAND_MIDDLE_FINGER_CONNECTIONS[3] = {std::pair(9, 10), std::pair(10, 11), std::pair(11, 12)};
constexpr int HAND_MIDDLE_FINGER_LANDMARKS[4] = {9, 10, 11, 12};
std::pair<int, int> HAND_RING_FINGER_CONNECTIONS[3] = {std::pair(13, 14), std::pair(14, 15), std::pair(15, 16)};
constexpr int HAND_RING_FINGER_LANDMARKS[4] = {13, 14, 15, 16};
std::pair<int, int> HAND_PINKY_FINGER_CONNECTIONS[3] = {std::pair(17, 18), std::pair(18, 19), std::pair(19, 20)};
constexpr int HAND_PINKY_FINGER_LANDMARKS[4] = {17, 18, 19, 20}; 
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

void writeToCSV(mediapipe::NormalizedLandmarkList inputs, mediapipe::NormalizedRect handRect, int gesture_type) {
    std::fstream fout;

    fout.open("//Users/negativetraffic/Documents/traindata2.csv", std::ios::out | std::ios::app);

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
            std::find(std::begin(HAND_THUMB_LANDMARKS), std::end(HAND_THUMB_LANDMARKS), i) 
                != std::end(HAND_THUMB_LANDMARKS)) {
            fout << ", " << inputs.landmark(i).z();
        }
    }
    fout << getLineInfos(HAND_THUMB_CONNECTIONS, 3, pts);
    fout << getLineInfos(HAND_PALM_CONNECTIONS, 6, pts);
    /*
    float line_vec[3][3];
    for (int i = 1; i < 4; i++) {
        line_vec[i-1][0] = pts[i-1][0] - pts[i][0];
        line_vec[i-1][1] = pts[i-1][1] - pts[i][1];        
        line_vec[i-1][2] = pts[i-1][2] - pts[i][2];
    }
    for (int i = 0; i < 3; i++) {
        //distance
        fout << ", " << sqrt(pow(line_vec[i][0], 2) + pow(line_vec[i][1], 2));
    }
    for (int i = 0; i < 2; i++) {
        float m1 = line_vec[i][0] / line_vec[i][1];
        float m2 = line_vec[i + 1][0] / line_vec[i + 1][1];
        //angle
        fout << ", " << atan((m1 - m2)/(1 + m1 * m2));
    }
    */

    /*
    for (int i = 0; i < inputs.landmark_size(); i++) {
        //translate x and y coords
        float shift_x = (inputs.landmark(i).x() - top_x);
        float shift_y = (inputs.landmark(i).y() - top_y);
        //rotate and normalize
        fout << ", " << (cos(handRect.rotation()) * shift_x + sin(handRect.rotation()) * shift_y) / handRect.width()
            << ", " << (cos(handRect.rotation()) * shift_y - sin(handRect.rotation()) * shift_x) / handRect.height()
            << ", " << inputs.landmark(i).z();
    }
    */
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
                /*
                std::cout << "lanmark size: " << landmark_list.at(0).landmark_size();
                std::cout << "landmarks: ";
                for (int i = 0; i < landmark_list.at(0).landmark_size(); i++) {
                    std::cout << "[ " << landmark_list.at(0).landmark(i).x() << ", " << landmark_list.at(0).landmark(i).y() << ", " << landmark_list.at(0).landmark(i).z() << "] ";
                }
                std::cout << std::endl;
                std::cout << "vector size: " << landmark_list.size() << std::endl;

                std::vector<mediapipe::ClassificationList> handedness_list = handedness.Get<std::vector<mediapipe::ClassificationList>>();
                for (int i = 0; i < handedness_list.size(); i++) {
                    for (int x = 0; x < handedness_list.at(i).classification_size(); x++) {
                        std:: cout << "index: " << handedness_list.at(i).classification(x).index() 
                            << ", score: " << handedness_list.at(i).classification(x).score()
                            << ", lable: " << handedness_list.at(i).classification(x).label()
                            << ", display name: " << handedness_list.at(i).classification(x).display_name() << std::endl;
                    }
                }
                */

                /*
                std::cout << "handedness: " << handedness.DebugTypeName() << std::endl;
                std::cout << "palm: " << palm.DebugTypeName() << std::endl;
                std::cout << "rois_landmarks: " << rois_landmarks.DebugTypeName() << std::endl;
                std::cout << "rois_palm: " << rois_palm.DebugTypeName() << std::endl;
                */
                std::vector<mediapipe::NormalizedRect> hand_rects = rois_palm.Get<std::vector<mediapipe::NormalizedRect>>();
                /*
                for (int i = 0; i < hand_rects.size(); i++) {
                    std::cout << "(" << hand_rects.at(i).x_center() << ", " << hand_rects.at(i).y_center() << ")" 
                        << "height: " << hand_rects.at(i).height() 
                        << "width: " << hand_rects.at(i).width()
                        << "rotation: " << hand_rects.at(i).rotation() << std::endl;
                }
                */
                int gesture_type = write.Get<int>();

                if (gesture_type != 0) {
                    writeToCSV(landmark_list.at(0), hand_rects.at(0), gesture_type);
                }
                
            }
            return OkStatus();
        }
    };

    REGISTER_CALCULATOR(CoutCalculator);
}