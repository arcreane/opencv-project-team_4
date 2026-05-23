#include "CannyFilter.h"
#include "ui_DragDropCannyEdge.h"
#include <opencv2/opencv.hpp>


CannyOperation::CannyOperation(Ui_CannyEdge* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}

void CannyOperation::showInterface() {
	ui->apertureBox->setVisible(true);
	ui->cannyFilterButton->setText("Back");
	ui->apertureLabel->setVisible(true);
	ui->lowerTslider->setVisible(true);
	ui->higherTslider->setVisible(true);
	ui->lowerTlabel->setVisible(true);
	ui->higherTlabel->setVisible(true);
	ui->applyCannyFilterButton->setVisible(true);
   
}

void CannyOperation::hideInterface() {
    ui->apertureBox->setVisible(false);
    ui->apertureLabel->setVisible(false);
    ui->lowerTslider->setVisible(false);
    ui->higherTslider->setVisible(false);
    ui->lowerTlabel->setVisible(false);
    ui->higherTlabel->setVisible(false);
	ui->resultCannyLabel->setVisible(false);
    ui->dragAndDrop->clear();
    ui->dragAndDrop->setText("Drop  your image here");
    ui->dragAndDrop->setStyleSheet("border: 2px dashed gray;");
    ui->applyCannyFilterButton->setVisible(false);
	ui->cannyFilterButton->setText("Canny Edge Filter");
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
