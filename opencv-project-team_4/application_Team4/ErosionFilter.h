#pragma once
#include <QObject>
#include <QImage>
#include <opencv2/opencv.hpp>

class Ui_Morphology;

class ErosionOperation : public QObject {
    Q_OBJECT
public:
    explicit ErosionOperation(Ui_Morphology* ui, QObject* parent = nullptr);
    void showInterface();
    void hideInterface();
	cv::Mat applyErosion(const cv::Mat& inputImage);
       

private:
    Ui_Morphology* ui;
}; 
