#include <string>
#include <iostream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

constexpr char FILE_PATH[] = "mediapipe/examples/subgraph_testing/graphs/hand_tracking.pbtxt";
constexpr char WINDOW_NAME[] = "camera";
constexpr char INPUT_STREAM[] = "input_video";
constexpr char OUTPUT_STREAM[] = "output_video";
constexpr char WRITE_FILE[] = "write";

absl::Status run() {
    std::string graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
        FILE_PATH, &graph_config_contents));

    mediapipe::CalculatorGraphConfig config = 
        mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
            graph_config_contents);

    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
        graph.AddOutputStreamPoller(OUTPUT_STREAM));
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    cv::VideoCapture cap(cv::CAP_ANY);
    cv::namedWindow(WINDOW_NAME);
    cv::Mat frame;
    int gesture_type_counter = 0;
    bool writeToCSV = false;
    int sampleCounter = 0;
    const int maxSample = 200;
    while(true) {
        cap >> frame;
        if (frame.empty()) {
            std::cout << "empty frame, camera disconnected" << std::endl;
            return absl::InternalError("camera disconnected");
        }
        cv::flip(frame, frame, 1);
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        auto converted_frame = absl::make_unique<mediapipe::ImageFrame>(
            mediapipe::ImageFormat::SRGB, frame.cols, frame.rows,
            mediapipe::ImageFrame::kDefaultAlignmentBoundary);
        cv::Mat frame_mat = mediapipe::formats::MatView(converted_frame.get());
        frame.copyTo(frame_mat);

        size_t frame_timestamp_us =
            (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(converted_frame.release())
            .At(mediapipe::Timestamp(frame_timestamp_us))));

        int write_gesture = 0;
        if (writeToCSV) {
            if (sampleCounter > maxSample) {
                sampleCounter = 0;
                writeToCSV = false;
                std::cout << "complete" << std::endl;
            } else {
                write_gesture = gesture_type_counter;
                sampleCounter++;
            }
        }            
        auto write_frame = absl::make_unique<int>(write_gesture);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            WRITE_FILE, mediapipe::Adopt(write_frame.release())
            .At(mediapipe::Timestamp(frame_timestamp_us))));

        mediapipe::Packet packet;
        if (!poller.Next(&packet)) {
            break;
        }

        auto& output_frame = packet.Get<mediapipe::ImageFrame>();
        cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
        cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
        cv::imshow(WINDOW_NAME, output_frame_mat);
        const int key = cv::waitKey(3);
        if (key == 27) {
            break;
        } else if (key == 32) {
            std::cout << "start or stop writing" << std::endl;
            writeToCSV = !writeToCSV;
            if(writeToCSV) {
                gesture_type_counter++;
            }
        } else if (key == 48) {
            //0
            std::cout << "start or stop writing" << std::endl;
            writeToCSV = !writeToCSV;
            gesture_type_counter = 32;
        } else if (key == 49) {
            //1
            std::cout << "start or stop writing" << std::endl;
            writeToCSV = !writeToCSV;
            gesture_type_counter = 1;
        } else if (key == 50) {
            //2
            std::cout << "start or stop writing" << std::endl;
            writeToCSV = !writeToCSV;
            gesture_type_counter = 2;
        } else if (key == 51) {
            //3
            std::cout << "start or stop writing" << std::endl;
            writeToCSV = !writeToCSV;
            gesture_type_counter = 3;
        }
        
    }
    MP_RETURN_IF_ERROR(graph.CloseInputStream(OUTPUT_STREAM));
    return graph.WaitUntilDone();
}

int main() {
    absl::Status status = run();
    std::cout << "status =" << status << std::endl;
    std::cout << "status.ok() = " << status.ok() << std::endl;

}