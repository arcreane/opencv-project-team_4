#pragma once
#include <QObject>
#include <QImage>
#include <opencv2/opencv.hpp>

class Ui_Morphology;

class OpeningOperation : public QObject {
    Q_OBJECT
public:
    explicit OpeningOperation(Ui_Morphology* ui, QObject* parent = nullptr);
    void showInterface();
    void hideInterface();
    cv::Mat applyOpening(const cv::Mat& inputImage);


private:
    Ui_Morphology* ui;
};
#pragma once
