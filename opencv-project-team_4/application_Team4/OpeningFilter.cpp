#include "OpeningFilter.h"
#include "ui_MorphologyWindow.h"
#include <opencv2/opencv.hpp>


OpeningOperation::OpeningOperation(Ui_Morphology* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}
//Function to show the opening filter interface, hiding other filter options 
// and displaying relevant controls for the opening operation.
void OpeningOperation::showInterface() {
    ui->DilationFilter->setVisible(false);
    ui->ErosionFilter->setVisible(false);
    ui->ClosingFilter->setVisible(false);
	ui->OpeningFilter->setText("Back");
	ui->LabelTitle->setText("Morphology - Opening Filter");
    ui->erosionTypeLabel->setText("Define the type of opening");
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
//Function to hide the opening filter interface, restoring the main morphology options
// and clearing any results or selections related to the opening operation.
void OpeningOperation::hideInterface() {
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
    ui->ErosionFilter->setVisible(true);
    ui->ClosingFilter->setVisible(true);
    ui->applyButton->setVisible(false);
	ui->OpeningFilter->setText("Opening Filter");
    ui->LabelTitle->setText("MORPHOLOGY");
	ui->resultLabel->clear();
	ui->label->clear();
	ui->savedButton->setVisible(false);

}
//function to apply the opening operation to the input image based on the user's selections for structuring element type,
cv::Mat OpeningOperation::applyOpening(const cv::Mat& inputImage) {
    ui->savedButton->setVisible(true);

    int openingType = 0;
    //definition of the type of SE
    if (ui->rectMorph->isChecked()) {
        openingType = cv::MORPH_RECT;
    }
    else if (ui->ellipseMorph->isChecked()) {
        openingType = cv::MORPH_ELLIPSE;
    }
    else if (ui->crossMorph->isChecked()) {
        openingType = cv::MORPH_CROSS;
    }
    int kernelSize = ui->kernelSizeBox->value();
    int iteration = ui->iterationNumberBox->value();
    //getting the structure element
    cv::Mat element = cv::getStructuringElement(openingType, cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1), cv::Point(kernelSize, kernelSize));
    cv::Mat outputImage;
    //apply the opening filter, ATTENTION: put the parameter MORPH_OPEN otherwise no opening is computed
    cv::morphologyEx(inputImage, outputImage, cv::MORPH_OPEN, element, cv::Point(-1, -1), iteration);

    return outputImage;
}
