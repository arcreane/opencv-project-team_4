#include "thresholding.hpp"
#include <opencv2/opencv.hpp>

static cv::Mat gray;
static int threshValue = 127;
static int threshMode  = 0; // 0=Binary, 1=Otsu, 2=Adaptive

static void applyThreshold() {
    cv::Mat result;

    if (threshMode == 1) {
        // Otsu — threshold value is computed automatically
        cv::threshold(gray, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    } else if (threshMode == 2) {
        // Adaptive — local thresholding, ignores the slider value
        cv::adaptiveThreshold(gray, result, 255,
                              cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY, 11, 2);
    } else {
        // Binary with manual threshold
        cv::threshold(gray, result, threshValue, 255, cv::THRESH_BINARY);
    }

    cv::imshow("Thresholding", result);
}

static void onThreshValue(int val, void*) {
    threshValue = val;
    applyThreshold();
}

static void onThreshMode(int val, void*) {
    threshMode = val;
    applyThreshold();
}

void runThresholding(const cv::Mat& src) {
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    cv::namedWindow("Thresholding", cv::WINDOW_NORMAL);
    cv::createTrackbar("Threshold", "Thresholding", &threshValue, 255, onThreshValue);
    cv::createTrackbar("Mode 0=Binary 1=Otsu 2=Adaptive", "Thresholding",
                       &threshMode, 2, onThreshMode);

    applyThreshold();
    cv::waitKey(0);
}
