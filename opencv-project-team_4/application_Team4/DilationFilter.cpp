#include "DilationFilter.h"
#include "ui_MorphologyWindow.h"
#include <opencv2/opencv.hpp>


DilationOperation::DilationOperation(Ui_Morphology* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}

//Function to show the dilation filter interface, hiding other filter options 
// and displaying relevant controls for the dilation operation.
void DilationOperation::showInterface() {
	ui->DilationFilter->setText("Back");
    ui->ErosionFilter->setVisible(false);
    ui->OpeningFilter->setVisible(false);
    ui->ClosingFilter->setVisible(false);
    ui->iterationNumberBox->setVisible(true);
	ui->LabelTitle->setText("Morphology - Dilation Filter");
	ui->erosionTypeLabel->setText("Define the type of dilation");
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


//Function to hide the dilation filter interface, restoring the main morphology options
// and clearing any results or selections related to the dilation operation.
void DilationOperation::hideInterface() {
    ui->iterationNumberBox->setVisible(false);
    ui->iterationLabel->setVisible(false);
    ui->rectMorph->setVisible(false);
    ui->ellipseMorph->setVisible(false);
    ui->crossMorph->setVisible(false);
    ui->erosionTypeLabel->setVisible(false);
    ui->resultLabel->setVisible(false);
    ui->kernelSizeLabel->setVisible(false);
    ui->kernelSizeBox->setVisible(false);
    ui->ErosionFilter->setVisible(true);
    ui->OpeningFilter->setVisible(true);
    ui->ClosingFilter->setVisible(true);
    ui->applyButton->setVisible(false);
	ui->DilationFilter->setText("Dilation Filter");
    ui->LabelTitle->setText("MORPHOLOGY");
    ui->resultLabel->clear();
    ui->label->clear();
    ui->savedButton->setVisible(false);

}

//function to apply the dilation operation to the input image based on the user's selections for structuring element type,
cv::Mat DilationOperation::applyDilation(const cv::Mat& inputImage) {
    ui->savedButton->setVisible(true);

    int dilationType = 0;
    //Type of structuring element
    if (ui->rectMorph->isChecked()) {
        dilationType = cv::MORPH_RECT;
    }
    else if (ui->ellipseMorph->isChecked()) {
        dilationType = cv::MORPH_ELLIPSE;
    }
    else if (ui->crossMorph->isChecked()) {
        dilationType = cv::MORPH_CROSS;
    }
    int kernelSize = ui->kernelSizeBox->value();
    int iteration = ui->iterationNumberBox->value();
	//get the structuring element based on the selected type and kernel size, then apply dilation to the input image using OpenCV's dilate function, returning the processed image.
    cv::Mat element = cv::getStructuringElement(dilationType, cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1), cv::Point(kernelSize, kernelSize));
    cv::Mat outputImage;

    //Compute the dilation
    cv::dilate(inputImage, outputImage, element, cv::Point(-1, -1), iteration);

    return outputImage;
}
