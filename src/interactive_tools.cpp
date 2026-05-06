#include "interactive_tools.hpp"
#include <opencv2/opencv.hpp>
#include <deque>
#include <iostream>

static const std::string WIN = "Interactive Tools";
static const int MAX_HISTORY = 20;

// --- State ---
static cv::Mat current;
static std::deque<cv::Mat> undoStack;
static std::deque<cv::Mat> redoStack;

// --- Crop state ---
static bool cropping   = false;
static bool cropDone   = false;
static cv::Point cropStart, cropEnd;

// --- Brush state ---
static bool brushActive = false;
static int  brushSize   = 10;
static cv::Vec3b brushColor(0, 0, 255); // red

// --- Mode: 0=view, 1=crop, 2=brush ---
static int toolMode = 0;

// ── History helpers ────────────────────────────────────────────────────────

static void pushUndo() {
    undoStack.push_back(current.clone());
    if (undoStack.size() > MAX_HISTORY)
        undoStack.pop_front();
    redoStack.clear();
}

static void undo() {
    if (undoStack.empty()) return;
    redoStack.push_back(current.clone());
    current = undoStack.back().clone();
    undoStack.pop_back();
    cv::imshow(WIN, current);
}

static void redo() {
    if (redoStack.empty()) return;
    undoStack.push_back(current.clone());
    current = redoStack.back().clone();
    redoStack.pop_back();
    cv::imshow(WIN, current);
}

// ── Crop ──────────────────────────────────────────────────────────────────

static void applyCrop() {
    int x1 = std::min(cropStart.x, cropEnd.x);
    int y1 = std::min(cropStart.y, cropEnd.y);
    int x2 = std::max(cropStart.x, cropEnd.x);
    int y2 = std::max(cropStart.y, cropEnd.y);

    x1 = std::max(0, x1); y1 = std::max(0, y1);
    x2 = std::min(current.cols - 1, x2);
    y2 = std::min(current.rows - 1, y2);

    if (x2 - x1 < 5 || y2 - y1 < 5) return;

    pushUndo();
    current = current(cv::Rect(x1, y1, x2 - x1, y2 - y1)).clone();
    cv::imshow(WIN, current);
}

// ── Mouse callback ─────────────────────────────────────────────────────────

static void onMouse(int event, int x, int y, int, void*) {
    // ── Crop mode ──
    if (toolMode == 1) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            cropStart = {x, y};
            cropping  = true;
            cropDone  = false;
        } else if (event == cv::EVENT_MOUSEMOVE && cropping) {
            cropEnd = {x, y};
            cv::Mat display = current.clone();
            cv::rectangle(display, cropStart, cropEnd, {0, 255, 0}, 2);
            cv::imshow(WIN, display);
        } else if (event == cv::EVENT_LBUTTONUP && cropping) {
            cropEnd  = {x, y};
            cropping = false;
            cropDone = true;
            applyCrop();
        }
        return;
    }

    // ── Brush mode ──
    if (toolMode == 2) {
        if (event == cv::EVENT_LBUTTONDOWN) {
            brushActive = true;
            pushUndo();
        } else if (event == cv::EVENT_LBUTTONUP) {
            brushActive = false;
        }
        if (brushActive && (event == cv::EVENT_MOUSEMOVE ||
                            event == cv::EVENT_LBUTTONDOWN)) {
            cv::circle(current, {x, y}, brushSize, brushColor, -1);
            cv::imshow(WIN, current);
        }
    }
}

// ── Tool mode trackbar ─────────────────────────────────────────────────────

static void onToolMode(int val, void*) {
    toolMode = val;
    std::string hint;
    if (val == 1) hint = "CROP: click and drag, release to crop";
    else if (val == 2) hint = "BRUSH: hold left click and draw";
    else hint = "VIEW: Ctrl+Z undo  Ctrl+Y redo";
    std::cout << "[Interactive Tools] Mode " << val << " — " << hint << std::endl;
    cv::imshow(WIN, current);
}

// ── Entry point ────────────────────────────────────────────────────────────

void runInteractiveTools(const cv::Mat& src) {
    // Downscale large images so the window fits the screen
    double scale = std::min(1.0, 800.0 / std::max(src.cols, src.rows));
    cv::resize(src, current, cv::Size(), scale, scale);

    cv::namedWindow(WIN, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(WIN, onMouse);

    cv::createTrackbar("Tool 0=View 1=Crop 2=Brush", WIN, nullptr, 2, onToolMode);
    cv::setTrackbarPos("Tool 0=View 1=Crop 2=Brush", WIN, 0);

    cv::imshow(WIN, current);
    std::cout << "[Interactive Tools] Ctrl+Z=undo  Ctrl+Y=redo  S=save  Q=quit" << std::endl;

    while (true) {
        int key = cv::waitKey(20);
        if (key == 'q' || key == 'Q' || key == 27) break;

        // Ctrl+Z
        if (key == 26) { undo(); continue; }
        // Ctrl+Y
        if (key == 25) { redo(); continue; }

        // S — save result
        if (key == 's' || key == 'S') {
            cv::imwrite("output.jpg", current);
            std::cout << "[Interactive Tools] Saved to output.jpg" << std::endl;
        }
    }

    cv::destroyWindow(WIN);
}
