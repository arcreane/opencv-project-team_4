#include "creative_effects.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>

static cv::Mat source;
static cv::Mat preview; // downscaled for fast processing
static int effectMode = 0;

static cv::Mat applyCartoon(const cv::Mat& img) {
    cv::Mat gray, edges, blurred, cartoon;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);
    cv::Laplacian(gray, edges, CV_8U, 3);
    cv::threshold(edges, edges, 80, 255, cv::THRESH_BINARY_INV);

    // Single bilateral filter pass — fast enough for preview
    cv::bilateralFilter(img, blurred, 9, 75, 75);

    cv::cvtColor(edges, edges, cv::COLOR_GRAY2BGR);
    cv::bitwise_and(blurred, edges, cartoon);
    return cartoon;
}

static cv::Mat applyPencilSketch(const cv::Mat& img) {
    cv::Mat sketch_gray, sketch_color;
    cv::pencilSketch(img, sketch_gray, sketch_color, 60, 0.07f, 0.02f);
    return sketch_gray;
}

static cv::Mat applyVignette(const cv::Mat& img) {
    int rows = img.rows, cols = img.cols;
    // Build vignette mask using Gaussian blur on a white circle
    cv::Mat mask(rows, cols, CV_32F, cv::Scalar(1.0f));
    cv::Point center(cols / 2, rows / 2);
    cv::circle(mask, center, (int)(std::min(rows, cols) * 0.55), cv::Scalar(0.0f), -1);
    cv::GaussianBlur(mask, mask, cv::Size(0, 0), std::min(rows, cols) * 0.3);
    cv::normalize(mask, mask, 0.15, 1.0, cv::NORM_MINMAX);

    cv::Mat result;
    img.convertTo(result, CV_32FC3);
    std::vector<cv::Mat> channels(3);
    cv::split(result, channels);
    for (auto& c : channels) c = c.mul(mask);
    cv::merge(channels, result);
    result.convertTo(result, CV_8UC3);
    return result;
}

static void applyEffect() {
    cv::Mat result;
    switch (effectMode) {
        case 0: result = applyCartoon(preview);      break;
        case 1: result = applyPencilSketch(preview); break;
        case 2: result = applyVignette(preview);     break;
        default: result = preview;
    }
    cv::imshow("Creative Effects", result);
}

static void onEffectMode(int val, void*) {
    effectMode = val;
    applyEffect();
}

void runCreativeEffects(const cv::Mat& src) {
    source = src.clone();
    // Downscale to max 640px wide for fast processing
    double scale = std::min(1.0, 640.0 / src.cols);
    cv::resize(source, preview, cv::Size(), scale, scale);

    cv::namedWindow("Creative Effects", cv::WINDOW_NORMAL);
    cv::createTrackbar("Mode 0=Cartoon 1=Sketch 2=Vignette",
                       "Creative Effects", nullptr, 2, onEffectMode);
    cv::setTrackbarPos("Mode 0=Cartoon 1=Sketch 2=Vignette", "Creative Effects", effectMode);

    applyEffect();
    cv::waitKey(0);
}
