#include "creative_effects.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>

static cv::Mat source;
static cv::Mat preview;
static int effectMode  = 0; // 0=Cartoon 1=Sketch 2=Vignette 3=FilmGrain 4=ColorTint
static int tintColor   = 0; // 0=Red 1=Green 2=Blue 3=Yellow 4=Purple
static int grainAmount = 30;

// ── Cartoon ───────────────────────────────────────────────────────────────

static cv::Mat applyCartoon(const cv::Mat& img) {
    cv::Mat gray, edges, blurred, cartoon;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);
    cv::Laplacian(gray, edges, CV_8U, 3);
    cv::threshold(edges, edges, 80, 255, cv::THRESH_BINARY_INV);
    cv::bilateralFilter(img, blurred, 9, 75, 75);
    cv::cvtColor(edges, edges, cv::COLOR_GRAY2BGR);
    cv::bitwise_and(blurred, edges, cartoon);
    return cartoon;
}

// ── Pencil Sketch ─────────────────────────────────────────────────────────

static cv::Mat applyPencilSketch(const cv::Mat& img) {
    cv::Mat sketch_gray, sketch_color;
    cv::pencilSketch(img, sketch_gray, sketch_color, 60, 0.07f, 0.02f);
    return sketch_gray;
}

// ── Vignette ──────────────────────────────────────────────────────────────

static cv::Mat applyVignette(const cv::Mat& img) {
    int rows = img.rows, cols = img.cols;
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

// ── Film Grain ────────────────────────────────────────────────────────────

static cv::Mat applyFilmGrain(const cv::Mat& img) {
    cv::Mat noise(img.size(), CV_8UC3);
    cv::randn(noise, 0, grainAmount);
    cv::Mat result;
    cv::add(img, noise, result, cv::noArray(), CV_8UC3);
    // Slight desaturation for vintage feel
    cv::Mat gray, blended;
    cv::cvtColor(result, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(gray, gray, cv::COLOR_GRAY2BGR);
    cv::addWeighted(result, 0.75, gray, 0.25, 0, blended);
    return blended;
}

// ── Color Tint ────────────────────────────────────────────────────────────

static cv::Mat applyColorTint(const cv::Mat& img) {
    // tintColor: 0=Red 1=Green 2=Blue 3=Yellow 4=Purple
    cv::Scalar tints[] = {
        {0,   0,   80},   // Red   (BGR)
        {0,   80,  0 },   // Green
        {80,  0,   0 },   // Blue
        {0,   60,  60},   // Yellow
        {60,  0,   60}    // Purple
    };
    cv::Mat tintLayer(img.size(), CV_8UC3, tints[tintColor]);
    cv::Mat result;
    cv::addWeighted(img, 0.75, tintLayer, 0.25, 0, result);
    return result;
}

// ── Apply & callbacks ─────────────────────────────────────────────────────

static void applyEffect() {
    cv::Mat result;
    switch (effectMode) {
        case 0: result = applyCartoon(preview);      break;
        case 1: result = applyPencilSketch(preview); break;
        case 2: result = applyVignette(preview);     break;
        case 3: result = applyFilmGrain(preview);    break;
        case 4: result = applyColorTint(preview);    break;
        default: result = preview;
    }
    cv::imshow("Creative Effects", result);
}

static void onEffectMode(int val, void*)  { effectMode  = val; applyEffect(); }
static void onTintColor(int val, void*)   { tintColor   = val; applyEffect(); }
static void onGrainAmount(int val, void*) { grainAmount = std::max(1, val); applyEffect(); }

// ── Entry point ───────────────────────────────────────────────────────────

void runCreativeEffects(const cv::Mat& src) {
    source = src.clone();
    double scale = std::min(1.0, 640.0 / src.cols);
    cv::resize(source, preview, cv::Size(), scale, scale);

    cv::namedWindow("Creative Effects", cv::WINDOW_NORMAL);
    cv::resizeWindow("Creative Effects", 700, 600);

    cv::createTrackbar("Mode 0=Cartoon 1=Sketch 2=Vignette 3=Grain 4=Tint",
                       "Creative Effects", nullptr, 4, onEffectMode);
    cv::setTrackbarPos("Mode 0=Cartoon 1=Sketch 2=Vignette 3=Grain 4=Tint",
                       "Creative Effects", effectMode);

    cv::createTrackbar("Tint 0=Red 1=Green 2=Blue 3=Yellow 4=Purple",
                       "Creative Effects", nullptr, 4, onTintColor);
    cv::setTrackbarPos("Tint 0=Red 1=Green 2=Blue 3=Yellow 4=Purple",
                       "Creative Effects", tintColor);

    cv::createTrackbar("Grain Amount", "Creative Effects", nullptr, 80, onGrainAmount);
    cv::setTrackbarPos("Grain Amount", "Creative Effects", grainAmount);

    applyEffect();
    cv::waitKey(0);
}
