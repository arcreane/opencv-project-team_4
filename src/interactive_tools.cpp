#include "interactive_tools.hpp"
#include <opencv2/opencv.hpp>
#include <deque>
#include <iostream>

static const std::string WIN = "Interactive Tools";
static const int MAX_HISTORY = 20;

static cv::Mat current;
static cv::Mat original;
static std::deque<cv::Mat> undoStack;
static std::deque<cv::Mat> redoStack;

static bool  cropping    = false;
static cv::Point cropStart, cropEnd;
static bool  brushActive = false;
static int   brushSize   = 10;
static int   brushColorIdx = 0;
static int   toolMode    = 0; // 0=View 1=Crop 2=Brush 3=FloodFill

// Sliders for view-mode adjustments
static int brightnessVal = 50;  // 0-100, 50=neutral
static int contrastVal   = 50;  // 0-100, 50=neutral
static int blurVal       = 0;   // 0=none
static int rotateVal     = 0;   // 0-360
static int flipMode      = 0;   // 0=none 1=H 2=V 3=Both

static cv::Scalar getBrushColor() {
    switch (brushColorIdx) {
        case 1:  return {0,255,0};
        case 2:  return {255,0,0};
        case 3:  return {0,0,0};
        case 4:  return {255,255,255};
        default: return {0,0,255};
    }
}

// ── History ───────────────────────────────────────────────────────────────

static void pushUndo() {
    undoStack.push_back(current.clone());
    if (undoStack.size() > MAX_HISTORY) undoStack.pop_front();
    redoStack.clear();
}

static void undo() {
    if (undoStack.empty()) { std::cout << "[Undo] Nothing\n"; return; }
    redoStack.push_back(current.clone());
    current = undoStack.back().clone();
    undoStack.pop_back();
    cv::imshow(WIN, current);
    std::cout << "[Undo] " << undoStack.size() << " left\n";
}

static void redo() {
    if (redoStack.empty()) { std::cout << "[Redo] Nothing\n"; return; }
    undoStack.push_back(current.clone());
    current = redoStack.back().clone();
    redoStack.pop_back();
    cv::imshow(WIN, current);
}

// ── Crop ──────────────────────────────────────────────────────────────────

static void applyCrop() {
    int x1 = std::max(0, std::min(cropStart.x, cropEnd.x));
    int y1 = std::max(0, std::min(cropStart.y, cropEnd.y));
    int x2 = std::min(current.cols-1, std::max(cropStart.x, cropEnd.x));
    int y2 = std::min(current.rows-1, std::max(cropStart.y, cropEnd.y));
    if (x2-x1 < 5 || y2-y1 < 5) return;
    pushUndo();
    current = current(cv::Rect(x1,y1,x2-x1,y2-y1)).clone();
    cv::imshow(WIN, current);
    std::cout << "[Crop] " << current.cols << "x" << current.rows << "\n";
}

// ── Flood Fill ────────────────────────────────────────────────────────────

static void applyFloodFill(int x, int y) {
    if (x < 0 || y < 0 || x >= current.cols || y >= current.rows) return;
    pushUndo();
    cv::Scalar fillColor = getBrushColor();
    cv::floodFill(current, {x, y}, fillColor, nullptr,
                  cv::Scalar(30,30,30), cv::Scalar(30,30,30));
    cv::imshow(WIN, current);
}

// ── Brightness / Contrast ─────────────────────────────────────────────────

static cv::Mat applyBrightnessContrast(const cv::Mat& img) {
    double alpha = 0.5 + (contrastVal / 50.0);   // 0.5 – 2.5
    int    beta  = (brightnessVal - 50) * 2;      // -100 – +100
    cv::Mat result;
    img.convertTo(result, -1, alpha, beta);
    return result;
}

// ── Rotation ──────────────────────────────────────────────────────────────

static cv::Mat applyRotation(const cv::Mat& img) {
    cv::Point2f center(img.cols/2.0f, img.rows/2.0f);
    cv::Mat M = cv::getRotationMatrix2D(center, rotateVal, 1.0);
    cv::Mat result;
    cv::warpAffine(img, result, M, img.size());
    return result;
}

// ── Flip ──────────────────────────────────────────────────────────────────

static cv::Mat applyFlip(const cv::Mat& img) {
    if (flipMode == 0) return img;
    cv::Mat result;
    int code = (flipMode == 1) ? 1 : (flipMode == 2) ? 0 : -1;
    cv::flip(img, result, code);
    return result;
}

// ── Blur ──────────────────────────────────────────────────────────────────

static cv::Mat applyBlur(const cv::Mat& img) {
    if (blurVal < 1) return img;
    int k = blurVal*2+1;
    cv::Mat result;
    cv::GaussianBlur(img, result, cv::Size(k,k), 0);
    return result;
}

// ── Composite preview (view mode) ─────────────────────────────────────────

static void showPreview() {
    cv::Mat img = current.clone();
    img = applyBrightnessContrast(img);
    img = applyBlur(img);
    img = applyRotation(img);
    img = applyFlip(img);
    cv::imshow(WIN, img);
}

// ── Mouse ─────────────────────────────────────────────────────────────────

static void onMouse(int event, int x, int y, int, void*) {
    if (toolMode == 1) { // Crop
        if (event == cv::EVENT_LBUTTONDOWN) { cropStart={x,y}; cropping=true; }
        else if (event == cv::EVENT_MOUSEMOVE && cropping) {
            cv::Mat d = current.clone();
            cv::rectangle(d, cropStart, {x,y}, {0,255,0}, 2);
            cv::imshow(WIN, d);
        } else if (event == cv::EVENT_LBUTTONUP && cropping) {
            cropEnd={x,y}; cropping=false; applyCrop();
        }
    } else if (toolMode == 2) { // Brush
        if (event == cv::EVENT_LBUTTONDOWN) { brushActive=true; pushUndo(); }
        else if (event == cv::EVENT_LBUTTONUP) { brushActive=false; }
        if (brushActive && (event==cv::EVENT_MOUSEMOVE||event==cv::EVENT_LBUTTONDOWN)) {
            cv::circle(current, {x,y}, brushSize, getBrushColor(), -1);
            cv::imshow(WIN, current);
        }
    } else if (toolMode == 3) { // Flood Fill
        if (event == cv::EVENT_LBUTTONDOWN) applyFloodFill(x, y);
    }
}

// ── Trackbar callbacks ────────────────────────────────────────────────────

static void onToolMode(int v, void*)    { toolMode      = v; cv::imshow(WIN, current); }
static void onBrushSize(int v, void*)   { brushSize     = std::max(1,v); }
static void onBrushColor(int v, void*)  { brushColorIdx = v; }
static void onBrightness(int v, void*)  { brightnessVal = v; showPreview(); }
static void onContrast(int v, void*)    { contrastVal   = v; showPreview(); }
static void onBlur(int v, void*)        { blurVal       = v; showPreview(); }
static void onRotate(int v, void*)      { rotateVal     = v; showPreview(); }
static void onFlip(int v, void*)        { flipMode      = v; showPreview(); }

// ── Entry point ───────────────────────────────────────────────────────────

void runInteractiveTools(const cv::Mat& src) {
    double scale = std::min(1.0, 800.0 / std::max(src.cols, src.rows));
    cv::resize(src, current, cv::Size(), scale, scale);
    original = current.clone();

    cv::namedWindow(WIN, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(WIN, onMouse);

    cv::createTrackbar("Tool 0=View 1=Crop 2=Brush 3=Fill", WIN, nullptr, 3, onToolMode);
    cv::setTrackbarPos("Tool 0=View 1=Crop 2=Brush 3=Fill", WIN, 0);

    cv::createTrackbar("Brush Size",  WIN, nullptr, 50, onBrushSize);
    cv::setTrackbarPos("Brush Size",  WIN, brushSize);

    cv::createTrackbar("Color 0=Red 1=Green 2=Blue 3=Black 4=White", WIN, nullptr, 4, onBrushColor);
    cv::setTrackbarPos("Color 0=Red 1=Green 2=Blue 3=Black 4=White", WIN, 0);

    cv::createTrackbar("Brightness", WIN, nullptr, 100, onBrightness);
    cv::setTrackbarPos("Brightness", WIN, brightnessVal);

    cv::createTrackbar("Contrast",   WIN, nullptr, 100, onContrast);
    cv::setTrackbarPos("Contrast",   WIN, contrastVal);

    cv::createTrackbar("Blur",       WIN, nullptr, 20,  onBlur);
    cv::setTrackbarPos("Blur",       WIN, blurVal);

    cv::createTrackbar("Rotate",     WIN, nullptr, 360, onRotate);
    cv::setTrackbarPos("Rotate",     WIN, rotateVal);

    cv::createTrackbar("Flip 0=None 1=H 2=V 3=Both", WIN, nullptr, 3, onFlip);
    cv::setTrackbarPos("Flip 0=None 1=H 2=V 3=Both", WIN, flipMode);

    cv::imshow(WIN, current);
    std::cout << "[Interactive Tools] R=reset  S=save  Ctrl+Z=undo  Ctrl+Y=redo  Q=quit\n";

    while (true) {
        int key = cv::waitKey(20);
        if (key=='q'||key=='Q'||key==27) break;
        if (key==26) { undo(); continue; }
        if (key==25) { redo(); continue; }
        if (key=='r'||key=='R') {
            pushUndo(); current=original.clone();
            cv::imshow(WIN, current);
            std::cout << "[Reset] Restored original\n";
        }
        if (key=='s'||key=='S') {
            cv::imwrite("output.jpg", current);
            std::cout << "[Save] output.jpg\n";
        }
    }
    cv::destroyWindow(WIN);
}
