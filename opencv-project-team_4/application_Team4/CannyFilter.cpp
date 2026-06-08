#include "CannyFilter.h"
#include "ui_DragDropCannyEdge.h"
#include <opencv2/opencv.hpp>


CannyOperation::CannyOperation(Ui_CannyEdge* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}

//Function to activate the interface of canny edge filter, 
//making the necessary widgets visible and changing the text of the button to "Back"
//to allow users to return to the previous interface.
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
//Function to deactivate the interface of canny edge filter, 
//making the necessary widgets invisible and changing the text of the button to "Canny Edge Filter"
//to allow users to return to the previous interface.
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
	ui->savedButton->setVisible(false);
}

//Function to apply the Canny edge filter to the input image using the parameters specified by the user through the interface.
//The parameters are the lower and higher thresholds for the hysteresis procedure and the aperture size for the Sobel operator. 
// The function returns the processed image with the detected edges.
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
    ui->savedButton->setVisible(true);

    return outputImage;
}
