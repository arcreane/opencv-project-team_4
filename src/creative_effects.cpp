#include "creative_effects.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>

static cv::Mat source;
static cv::Mat preview;
static int effectMode  = 0; // 0=Cartoon 1=Sketch 2=Vignette 3=Grain 4=Tint 5=Sepia 6=Emboss 7=Pixelate 8=Negative 9=Warm/Cool
static int tintColor   = 0;
static int grainAmount = 30;
static int pixelSize   = 10;
static int warmCool    = 50; // 0=very cool, 50=neutral, 100=very warm

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
    cv::circle(mask, {cols/2, rows/2}, (int)(std::min(rows,cols)*0.55), cv::Scalar(0.0f), -1);
    cv::GaussianBlur(mask, mask, cv::Size(0,0), std::min(rows,cols)*0.3);
    cv::normalize(mask, mask, 0.15, 1.0, cv::NORM_MINMAX);
    cv::Mat result;
    img.convertTo(result, CV_32FC3);
    std::vector<cv::Mat> ch(3);
    cv::split(result, ch);
    for (auto& c : ch) c = c.mul(mask);
    cv::merge(ch, result);
    result.convertTo(result, CV_8UC3);
    return result;
}

// ── Film Grain ────────────────────────────────────────────────────────────

static cv::Mat applyFilmGrain(const cv::Mat& img) {
    cv::Mat noise(img.size(), CV_8UC3);
    cv::randn(noise, 0, grainAmount);
    cv::Mat result;
    cv::add(img, noise, result, cv::noArray(), CV_8UC3);
    cv::Mat gray, blended;
    cv::cvtColor(result, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(gray, gray, cv::COLOR_GRAY2BGR);
    cv::addWeighted(result, 0.75, gray, 0.25, 0, blended);
    return blended;
}

// ── Color Tint ────────────────────────────────────────────────────────────

static cv::Mat applyColorTint(const cv::Mat& img) {
    cv::Scalar tints[] = {
        {0,0,80}, {0,80,0}, {80,0,0}, {0,60,60}, {60,0,60}
    };
    cv::Mat tintLayer(img.size(), CV_8UC3, tints[tintColor]);
    cv::Mat result;
    cv::addWeighted(img, 0.75, tintLayer, 0.25, 0, result);
    return result;
}

// ── Sepia ─────────────────────────────────────────────────────────────────

static cv::Mat applySepia(const cv::Mat& img) {
    cv::Mat result = img.clone();
    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            cv::Vec3b p = img.at<cv::Vec3b>(y, x);
            int b = p[0], g = p[1], r = p[2];
            result.at<cv::Vec3b>(y, x) = cv::Vec3b(
                cv::saturate_cast<uchar>(b*0.131 + g*0.534 + r*0.272),
                cv::saturate_cast<uchar>(b*0.168 + g*0.686 + r*0.349),
                cv::saturate_cast<uchar>(b*0.189 + g*0.769 + r*0.393)
            );
        }
    }
    return result;
}

// ── Emboss ────────────────────────────────────────────────────────────────

static cv::Mat applyEmboss(const cv::Mat& img) {
    cv::Mat gray, result;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
        -2, -1, 0,
        -1,  1, 1,
         0,  1, 2);
    cv::filter2D(gray, result, CV_32F, kernel);
    result += 128;
    result.convertTo(result, CV_8U);
    cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    return result;
}

// ── Pixelate ──────────────────────────────────────────────────────────────

static cv::Mat applyPixelate(const cv::Mat& img) {
    int ps = std::max(2, pixelSize);
    cv::Mat small, result;
    cv::resize(img, small, cv::Size(img.cols/ps, img.rows/ps), 0, 0, cv::INTER_LINEAR);
    cv::resize(small, result, img.size(), 0, 0, cv::INTER_NEAREST);
    return result;
}

// ── Negative ──────────────────────────────────────────────────────────────

static cv::Mat applyNegative(const cv::Mat& img) {
    cv::Mat result;
    cv::bitwise_not(img, result);
    return result;
}

// ── Warm / Cool ───────────────────────────────────────────────────────────

static cv::Mat applyWarmCool(const cv::Mat& img) {
    cv::Mat result = img.clone();
    int shift = warmCool - 50; // -50 (cool) to +50 (warm)
    std::vector<cv::Mat> ch;
    cv::split(result, ch);
    // Warm: boost red, reduce blue / Cool: boost blue, reduce red
    ch[2].convertTo(ch[2], -1, 1.0, shift);    // R
    ch[0].convertTo(ch[0], -1, 1.0, -shift);   // B
    cv::merge(ch, result);
    return result;
}

// ── Apply & callbacks ─────────────────────────────────────────────────────

static void applyEffect() {
    cv::Mat result;
    switch (effectMode) {
        case 0: result = applyCartoon(preview);     break;
        case 1: result = applyPencilSketch(preview);break;
        case 2: result = applyVignette(preview);    break;
        case 3: result = applyFilmGrain(preview);   break;
        case 4: result = applyColorTint(preview);   break;
        case 5: result = applySepia(preview);       break;
        case 6: result = applyEmboss(preview);      break;
        case 7: result = applyPixelate(preview);    break;
        case 8: result = applyNegative(preview);    break;
        case 9: result = applyWarmCool(preview);    break;
        default: result = preview;
    }
    cv::imshow("Creative Effects", result);
}

static void onEffectMode(int v, void*)  { effectMode  = v; applyEffect(); }
static void onTintColor(int v, void*)   { tintColor   = v; applyEffect(); }
static void onGrainAmount(int v, void*) { grainAmount = std::max(1,v); applyEffect(); }
static void onPixelSize(int v, void*)   { pixelSize   = std::max(2,v); applyEffect(); }
static void onWarmCool(int v, void*)    { warmCool    = v; applyEffect(); }

// ── Entry point ───────────────────────────────────────────────────────────

void runCreativeEffects(const cv::Mat& src) {
    source = src.clone();
    double scale = std::min(1.0, 640.0 / src.cols);
    cv::resize(source, preview, cv::Size(), scale, scale);

    cv::namedWindow("Creative Effects", cv::WINDOW_NORMAL);
    cv::resizeWindow("Creative Effects", 700, 650);

    cv::createTrackbar("Mode 0=Cartoon 1=Sketch 2=Vignette 3=Grain 4=Tint 5=Sepia 6=Emboss 7=Pixel 8=Neg 9=Warm",
                       "Creative Effects", nullptr, 9, onEffectMode);
    cv::setTrackbarPos("Mode 0=Cartoon 1=Sketch 2=Vignette 3=Grain 4=Tint 5=Sepia 6=Emboss 7=Pixel 8=Neg 9=Warm",
                       "Creative Effects", effectMode);

    cv::createTrackbar("Tint 0=Red 1=Green 2=Blue 3=Yellow 4=Purple",
                       "Creative Effects", nullptr, 4, onTintColor);
    cv::setTrackbarPos("Tint 0=Red 1=Green 2=Blue 3=Yellow 4=Purple",
                       "Creative Effects", tintColor);

    cv::createTrackbar("Grain Amount", "Creative Effects", nullptr, 80, onGrainAmount);
    cv::setTrackbarPos("Grain Amount", "Creative Effects", grainAmount);

    cv::createTrackbar("Pixel Size", "Creative Effects", nullptr, 40, onPixelSize);
    cv::setTrackbarPos("Pixel Size", "Creative Effects", pixelSize);

    cv::createTrackbar("Warm(100) <-> Cool(0)", "Creative Effects", nullptr, 100, onWarmCool);
    cv::setTrackbarPos("Warm(100) <-> Cool(0)", "Creative Effects", warmCool);

    applyEffect();
    cv::waitKey(0);
}
