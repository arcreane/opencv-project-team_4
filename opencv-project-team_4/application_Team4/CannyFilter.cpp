#include "CannyFilter.h"
#include "ui_DragDropCannyEdge.h"
#include <opencv2/opencv.hpp>


CannyOperation::CannyOperation(Ui_CannyEdge* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}

void CannyOperation::showInterface() {
    ui->cannyFilterButton->move(20, 90);
	ui->apertureBox->setVisible(true);
	ui->apertureLabel->setVisible(true);
	ui->lowerTslider->setVisible(true);
	ui->higherTslider->setVisible(true);
	ui->lowerTlabel->setVisible(true);
	ui->higherTlabel->setVisible(true);
	ui->applyCannyFilterButton->setVisible(true);
   
}

void CannyOperation::hideInterface() {
    ui->cannyFilterButton->move(70, 230);
    ui->apertureBox->setVisible(false);
    ui->apertureLabel->setVisible(false);
    ui->lowerTslider->setVisible(false);
    ui->higherTslider->setVisible(false);
    ui->lowerTlabel->setVisible(false);
    ui->higherTlabel->setVisible(false);
    ui->applyCannyFilterButton->setVisible(false);
}

cv::Mat CannyOperation::applyCannyEdgeFilter(const cv::Mat& inputImage) {

    int lowerThreshold = ui->lowerTslider->value();
    int higherThreshold = ui->higherTslider->value();
    int apertureSize = ui->apertureBox->value();
    cv::Mat outputImage;
    if (apertureSize == 0) {
        cv::Canny(inputImage, outputImage, lowerThreshold, higherThreshold, apertureSize);
    }
    else{
        cv::Canny(inputImage, outputImage, lowerThreshold, higherThreshold);
    }

    return outputImage;
}
