#include "ClosingFilter.h"
#include "ui_MorphologyWindow.h"
#include <opencv2/opencv.hpp>


ClosingOperation::ClosingOperation(Ui_Morphology* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}
//Function to show the closing filter interface, hiding other filter options 
// and displaying relevant controls for the closing operation.
void ClosingOperation::showInterface() {
    ui->DilationFilter->setVisible(false);
    ui->OpeningFilter->setVisible(false);
    ui->ErosionFilter->setVisible(false);
    ui->ClosingFilter->setText("Back");
	ui->LabelTitle->setText("Morphology - Closing Filter");
	ui->erosionTypeLabel->setText("Define the type of closing");
    ui->iterationNumberBox->setVisible(true);
    ui->iterationLabel->setVisible(true);
    ui->rectMorph->setVisible(true);
    ui->rectMorph->setToolTip("Rectangular SE — general purpose.");
    ui->ellipseMorph->setVisible(true);
    ui->ellipseMorph->setToolTip("Elliptical SE — rounded shapes.");
    ui->crossMorph->setVisible(true);
    ui->crossMorph->setToolTip("Cross SE — preserves thin structures.");
    ui->erosionTypeLabel->setVisible(true);
    ui->kernelSizeLabel->setVisible(true);
    ui->kernelSizeBox->setVisible(true);
    ui->applyButton->setVisible(true);
}
//Function to hide the closing filter interface, restoring the main morphology options
// and clearing any results or selections related to the closing operation.
void ClosingOperation::hideInterface() {
    ui->iterationNumberBox->setVisible(false);
    ui->iterationLabel->setVisible(false);
    ui->rectMorph->setVisible(false);
    ui->ellipseMorph->setVisible(false);
    ui->crossMorph->setVisible(false);
    ui->erosionTypeLabel->setVisible(false);
    ui->resultLabel->setVisible(false);
    ui->kernelSizeLabel->setVisible(false);
    ui->kernelSizeBox->setVisible(false);
    ui->DilationFilter->setVisible(true);
    ui->OpeningFilter->setVisible(true);
    ui->ErosionFilter->setVisible(true);
    ui->applyButton->setVisible(false);
    ui->LabelTitle->setText("MORPHOLOGY");
	ui->ClosingFilter->setText("Closing Filter");
    ui->resultLabel->clear();
    ui->label->clear();
    ui->savedButton->setVisible(false);
}

//Function to apply the closing operation to the input image based on the user's selections for structuring element type, 
// kernel size, and number of iterations
// The function uses OpenCV's morphologyEx function to perform the closing operation and returns the processed image.

cv::Mat ClosingOperation::applyClosing(const cv::Mat& inputImage) {
	ui->savedButton->setVisible(true);
    int closingType = 0;

    //type of structuring element
    if (ui->rectMorph->isChecked()) {
        closingType = cv::MORPH_RECT;
    }
    else if (ui->ellipseMorph->isChecked()) {
        closingType = cv::MORPH_ELLIPSE;
    }
    else if (ui->crossMorph->isChecked()) {
        closingType = cv::MORPH_CROSS;
    }
    int kernelSize = ui->kernelSizeBox->value();
    int iteration = ui->iterationNumberBox->value();

    //creation of the structuring element
    cv::Mat element = cv::getStructuringElement(closingType, cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1), cv::Point(kernelSize, kernelSize));
    cv::Mat outputImage;

	//applying the closing operation using the specified parameters: IMPORTANT : MORPH_CLOSE
    cv::morphologyEx(inputImage, outputImage, cv::MORPH_CLOSE, element, cv::Point(-1, -1), iteration);

    return outputImage;
}
