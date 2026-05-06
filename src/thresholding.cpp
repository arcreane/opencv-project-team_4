#include "thresholding.hpp"
#include <opencv2/opencv.hpp>

static cv::Mat gray;
static cv::Mat original;
static int threshValue = 127;
static int threshMode  = 0; // 0=Binary, 1=Otsu, 2=Adaptive
static int blockSize   = 11; // Adaptive block size (must be odd >= 3)

static void applyThreshold() {
    cv::Mat result;

    if (threshMode == 1) {
        cv::threshold(gray, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    } else if (threshMode == 2) {
        int bs = blockSize % 2 == 0 ? blockSize + 1 : blockSize;
        bs = std::max(3, bs);
        cv::adaptiveThreshold(gray, result, 255,
                              cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY, bs, 2);
    } else {
        cv::threshold(gray, result, threshValue, 255, cv::THRESH_BINARY);
    }

    // Side-by-side: original (gray) on left, result on right
    cv::Mat grayBGR;
    cv::cvtColor(gray, grayBGR, cv::COLOR_GRAY2BGR);
    cv::Mat resultBGR;
    cv::cvtColor(result, resultBGR, cv::COLOR_GRAY2BGR);

    cv::Mat sideBySide;
    cv::hconcat(grayBGR, resultBGR, sideBySide);

    // Labels
    cv::putText(sideBySide, "Original", {10, 30},
                cv::FONT_HERSHEY_SIMPLEX, 0.8, {0, 200, 0}, 2);
    cv::putText(sideBySide, "Threshold", {grayBGR.cols + 10, 30},
                cv::FONT_HERSHEY_SIMPLEX, 0.8, {0, 200, 0}, 2);

    cv::imshow("Thresholding", sideBySide);
}

static void onThreshValue(int val, void*) { threshValue = val; applyThreshold(); }
static void onThreshMode(int val, void*)  { threshMode  = val; applyThreshold(); }
static void onBlockSize(int val, void*)   { blockSize   = val; applyThreshold(); }

void runThresholding(const cv::Mat& src) {
    // Downscale for speed
    double scale = std::min(1.0, 640.0 / src.cols);
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(), scale, scale);
    cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);

    cv::namedWindow("Thresholding", cv::WINDOW_NORMAL);
    cv::resizeWindow("Thresholding", 1200, 500);

    cv::createTrackbar("Threshold", "Thresholding", nullptr, 255, onThreshValue);
    cv::setTrackbarPos("Threshold", "Thresholding", threshValue);

    cv::createTrackbar("Mode 0=Binary 1=Otsu 2=Adaptive", "Thresholding", nullptr, 2, onThreshMode);
    cv::setTrackbarPos("Mode 0=Binary 1=Otsu 2=Adaptive", "Thresholding", threshMode);

    cv::createTrackbar("Block Size (Adaptive)", "Thresholding", nullptr, 51, onBlockSize);
    cv::setTrackbarPos("Block Size (Adaptive)", "Thresholding", blockSize);

    applyThreshold();
    cv::waitKey(0);
}
