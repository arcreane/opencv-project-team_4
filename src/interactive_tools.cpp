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

static bool  cropping   = false;
static cv::Point cropStart, cropEnd;

static bool  brushActive = false;
static int   brushSize   = 10;
static int   brushColorIdx = 0; // 0=Red 1=Green 2=Blue 3=Black 4=White

static int   toolMode    = 0; // 0=View 1=Crop 2=Brush

static cv::Scalar getBrushColor() {
    switch (brushColorIdx) {
        case 1:  return {0, 255, 0};    // Green
        case 2:  return {255, 0, 0};    // Blue
        case 3:  return {0, 0, 0};      // Black
        case 4:  return {255, 255, 255};// White
        default: return {0, 0, 255};    // Red
    }
}

// ── History ───────────────────────────────────────────────────────────────

static void pushUndo() {
    undoStack.push_back(current.clone());
    if (undoStack.size() > MAX_HISTORY) undoStack.pop_front();
    redoStack.clear();
}

static void undo() {
    if (undoStack.empty()) { std::cout << "[Undo] Nothing to undo\n"; return; }
    redoStack.push_back(current.clone());
    current = undoStack.back().clone();
    undoStack.pop_back();
    cv::imshow(WIN, current);
    std::cout << "[Undo] " << undoStack.size() << " steps left\n";
}

static void redo() {
    if (redoStack.empty()) { std::cout << "[Redo] Nothing to redo\n"; return; }
    undoStack.push_back(current.clone());
    current = redoStack.back().clone();
    redoStack.pop_back();
    cv::imshow(WIN, current);
}

// ── Crop ──────────────────────────────────────────────────────────────────

static void applyCrop() {
    int x1 = std::max(0, std::min(cropStart.x, cropEnd.x));
    int y1 = std::max(0, std::min(cropStart.y, cropEnd.y));
    int x2 = std::min(current.cols - 1, std::max(cropStart.x, cropEnd.x));
    int y2 = std::min(current.rows - 1, std::max(cropStart.y, cropEnd.y));
    if (x2 - x1 < 5 || y2 - y1 < 5) return;
    pushUndo();
    current = current(cv::Rect(x1, y1, x2 - x1, y2 - y1)).clone();
    cv::imshow(WIN, current);
    std::cout << "[Crop] " << current.cols << "x" << current.rows << "\n";
}

// ── Mouse ─────────────────────────────────────────────────────────────────

static void onMouse(int event, int x, int y, int, void*) {
    if (toolMode == 1) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            cropStart = {x, y}; cropping = true;
        } else if (event == cv::EVENT_MOUSEMOVE && cropping) {
            cv::Mat display = current.clone();
            cv::rectangle(display, cropStart, {x, y}, {0, 255, 0}, 2);
            cv::imshow(WIN, display);
        } else if (event == cv::EVENT_LBUTTONUP && cropping) {
            cropEnd = {x, y}; cropping = false;
            applyCrop();
        }
        return;
    }

    if (toolMode == 2) {
        if (event == cv::EVENT_LBUTTONDOWN) { brushActive = true; pushUndo(); }
        else if (event == cv::EVENT_LBUTTONUP) { brushActive = false; }
        if (brushActive && (event == cv::EVENT_MOUSEMOVE ||
                            event == cv::EVENT_LBUTTONDOWN)) {
            cv::circle(current, {x, y}, brushSize, getBrushColor(), -1);
            cv::imshow(WIN, current);
        }
    }
}

// ── Trackbar callbacks ────────────────────────────────────────────────────

static void onToolMode(int val, void*) {
    toolMode = val;
    const char* hints[] = {
        "VIEW — Ctrl+Z undo  Ctrl+Y redo  R reset  S save",
        "CROP — click and drag to select area",
        "BRUSH — hold left click to draw"
    };
    std::cout << "[Tool] " << hints[val] << "\n";
    cv::imshow(WIN, current);
}

static void onBrushSize(int val, void*)  { brushSize = std::max(1, val); }
static void onBrushColor(int val, void*) { brushColorIdx = val; }

// ── Entry point ───────────────────────────────────────────────────────────

void runInteractiveTools(const cv::Mat& src) {
    double scale = std::min(1.0, 800.0 / std::max(src.cols, src.rows));
    cv::resize(src, current, cv::Size(), scale, scale);
    original = current.clone();

    cv::namedWindow(WIN, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(WIN, onMouse);

    cv::createTrackbar("Tool 0=View 1=Crop 2=Brush", WIN, nullptr, 2, onToolMode);
    cv::setTrackbarPos("Tool 0=View 1=Crop 2=Brush", WIN, 0);

    cv::createTrackbar("Brush Size", WIN, nullptr, 50, onBrushSize);
    cv::setTrackbarPos("Brush Size", WIN, brushSize);

    cv::createTrackbar("Color 0=Red 1=Green 2=Blue 3=Black 4=White",
                       WIN, nullptr, 4, onBrushColor);
    cv::setTrackbarPos("Color 0=Red 1=Green 2=Blue 3=Black 4=White", WIN, 0);

    cv::imshow(WIN, current);
    std::cout << "[Interactive Tools] R=reset  S=save  Ctrl+Z=undo  Ctrl+Y=redo  Q=quit\n";

    while (true) {
        int key = cv::waitKey(20);
        if (key == 'q' || key == 'Q' || key == 27) break;
        if (key == 26) { undo(); continue; }   // Ctrl+Z
        if (key == 25) { redo(); continue; }   // Ctrl+Y
        if (key == 'r' || key == 'R') {        // Reset
            pushUndo();
            current = original.clone();
            cv::imshow(WIN, current);
            std::cout << "[Reset] Restored original\n";
        }
        if (key == 's' || key == 'S') {
            cv::imwrite("output.jpg", current);
            std::cout << "[Save] Saved to output.jpg\n";
        }
    }

    cv::destroyWindow(WIN);
}
