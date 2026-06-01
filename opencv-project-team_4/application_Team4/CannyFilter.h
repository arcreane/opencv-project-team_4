#pragma once
#include <QObject>
#include <QImage>
#include <opencv2/opencv.hpp>

class Ui_CannyEdge;

class CannyOperation : public QObject {
    Q_OBJECT
public:
    explicit CannyOperation(Ui_CannyEdge* ui, QObject* parent = nullptr);
    void showInterface();
    void hideInterface();
    cv::Mat applyCannyEdgeFilter(const cv::Mat& inputImage);


private:
    Ui_CannyEdge* ui;
};
#pragma once
