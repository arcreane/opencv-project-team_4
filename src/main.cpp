#include <opencv2/opencv.hpp>
#include <iostream>
#include "thresholding.hpp"
#include "creative_effects.hpp"
#include "interactive_tools.hpp"

int main(int argc, char** argv) {
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;

    std::string imagePath = (argc > 1) ? argv[1] : "sample.jpg";
    cv::Mat img = cv::imread(imagePath);

    if (img.empty()) {
        std::cerr << "No image found at: " << imagePath << std::endl;
        std::cerr << "Usage: MyImageEditor <image_path>" << std::endl;
        return -1;
    }

    cv::imshow("Original", img);
    runThresholding(img);
    runCreativeEffects(img);
    runInteractiveTools(img);

    return 0;
}
