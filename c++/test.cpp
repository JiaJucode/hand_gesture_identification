#include <string>
#include <iostream>
#include <vector>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

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

constexpr char FILE_PATH[] = "mediapipe/examples/c++/graphs/model_testing.pbtxt";
constexpr char WINDOW_NAME[] = "camera";
constexpr char INPUT_STREAM[] = "input_video";
constexpr char OUTPUT_STREAM[] = "prediction";
constexpr char SIDE_INPUT_PACKET[] = "send_sock";

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
    int count = 0;
    while(true) {
        cap >> frame;        
        cv::imshow(WINDOW_NAME, frame);
        const int key = cv::waitKey(250);
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

        size_t frame_timestamp_us = count;
        count++;
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            INPUT_STREAM, mediapipe::Adopt(converted_frame.release())
            .At(mediapipe::Timestamp(frame_timestamp_us))));

        mediapipe::Packet packet;
        if (!poller.Next(&packet)) {
            break;
        }

        std::vector<float> result = packet.Get<std::vector<float>>();
        for (auto v: result) {
            std::cout << v << ", ";
        }
        std::cout << std::endl;
        if (key == 27) {
            break;
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