#include "thresholding.hpp"
#include <opencv2/opencv.hpp>

static cv::Mat gray;
static int threshValue  = 127;
static int threshMode   = 0;  // 0=Binary 1=BinaryInv 2=Trunc 3=ToZero 4=ToZeroInv 5=Otsu 6=AdaptGauss 7=AdaptMean
static int blockSize    = 11;
static int constC       = 2;
static int invertResult = 0;

static void applyThreshold() {
    cv::Mat result;
    int bs = (blockSize % 2 == 0) ? blockSize + 1 : blockSize;
    bs = std::max(3, bs);

    switch (threshMode) {
        case 0: cv::threshold(gray, result, threshValue, 255, cv::THRESH_BINARY);    break;
        case 1: cv::threshold(gray, result, threshValue, 255, cv::THRESH_BINARY_INV); break;
        case 2: cv::threshold(gray, result, threshValue, 255, cv::THRESH_TRUNC);     break;
        case 3: cv::threshold(gray, result, threshValue, 255, cv::THRESH_TOZERO);    break;
        case 4: cv::threshold(gray, result, threshValue, 255, cv::THRESH_TOZERO_INV);break;
        case 5: cv::threshold(gray, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU); break;
        case 6: cv::adaptiveThreshold(gray, result, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                    cv::THRESH_BINARY, bs, constC); break;
        case 7: cv::adaptiveThreshold(gray, result, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                    cv::THRESH_BINARY, bs, constC); break;
        default: result = gray;
    }

    if (invertResult) cv::bitwise_not(result, result);

    // Side-by-side display
    cv::Mat grayBGR, resultBGR, sideBySide;
    cv::cvtColor(gray, grayBGR, cv::COLOR_GRAY2BGR);
    cv::cvtColor(result, resultBGR, cv::COLOR_GRAY2BGR);
    cv::hconcat(grayBGR, resultBGR, sideBySide);
    cv::putText(sideBySide, "Original",  {10, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,200,0}, 2);
    cv::putText(sideBySide, "Threshold", {grayBGR.cols+10, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,200,0}, 2);
    cv::imshow("Thresholding", sideBySide);
}

static void onThreshValue(int v, void*)  { threshValue  = v; applyThreshold(); }
static void onThreshMode(int v, void*)   { threshMode   = v; applyThreshold(); }
static void onBlockSize(int v, void*)    { blockSize    = v; applyThreshold(); }
static void onConstC(int v, void*)       { constC       = v; applyThreshold(); }
static void onInvert(int v, void*)       { invertResult = v; applyThreshold(); }

void runThresholding(const cv::Mat& src) {
    double scale = std::min(1.0, 640.0 / src.cols);
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(), scale, scale);
    cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);

    cv::namedWindow("Thresholding", cv::WINDOW_NORMAL);
    cv::resizeWindow("Thresholding", 1280, 520);

    cv::createTrackbar("Threshold", "Thresholding", nullptr, 255, onThreshValue);
    cv::setTrackbarPos("Threshold", "Thresholding", threshValue);

    cv::createTrackbar("Mode 0=Bin 1=BinInv 2=Trunc 3=Zero 4=ZeroInv 5=Otsu 6=AdaptG 7=AdaptM",
                       "Thresholding", nullptr, 7, onThreshMode);
    cv::setTrackbarPos("Mode 0=Bin 1=BinInv 2=Trunc 3=Zero 4=ZeroInv 5=Otsu 6=AdaptG 7=AdaptM",
                       "Thresholding", threshMode);

    cv::createTrackbar("Block Size (Adaptive)", "Thresholding", nullptr, 51, onBlockSize);
    cv::setTrackbarPos("Block Size (Adaptive)", "Thresholding", blockSize);

    cv::createTrackbar("C Constant (Adaptive)", "Thresholding", nullptr, 20, onConstC);
    cv::setTrackbarPos("C Constant (Adaptive)", "Thresholding", constC);

    cv::createTrackbar("Invert Result", "Thresholding", nullptr, 1, onInvert);
    cv::setTrackbarPos("Invert Result", "Thresholding", invertResult);

    applyThreshold();
    cv::waitKey(0);
}
