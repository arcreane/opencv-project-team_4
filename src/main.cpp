#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    cv::Mat img = cv::Mat::zeros(480, 640, CV_8UC3);
    cv::putText(img, "MyEditor - OK", {50, 240},
                cv::FONT_HERSHEY_SIMPLEX, 1.5, {0, 255, 0}, 2);
    cv::imshow("MyEditor", img);
    cv::waitKey(0);
    return 0;
}