#pragma once
#include <QObject>
#include <QImage>
#include <opencv2/opencv.hpp>

class Ui_Morphology;

class DilationOperation : public QObject {
    Q_OBJECT
public:
    explicit DilationOperation(Ui_Morphology* ui, QObject* parent = nullptr);
    void showInterface();
    void hideInterface();
    cv::Mat applyDilation(const cv::Mat& inputImage);


private:
    Ui_Morphology* ui;
};
#pragma once
