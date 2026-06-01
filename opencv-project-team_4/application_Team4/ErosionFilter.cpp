#include "ErosionFilter.h"
#include "ui_MorphologyWindow.h"
#include <opencv2/opencv.hpp>


ErosionOperation::ErosionOperation(Ui_Morphology* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}

void ErosionOperation::showInterface() {
    ui->DilationFilter->setVisible(false);
    ui->OpeningFilter->setVisible(false);
    ui->ClosingFilter->setVisible(false);
    ui->iterationNumberBox->setVisible(true);
	ui->LabelTitle->setText("Morphology - Erosion Filter");
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
    ui->ErosionFilter->setText("Back");
}

void ErosionOperation::hideInterface() {
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
    ui->ClosingFilter->setVisible(true);
	ui->applyButton->setVisible(false);
    ui->ErosionFilter->setText("Erosion Filter");
	ui->LabelTitle->setText("MORPHOLOGY");
    ui->resultLabel->clear();
    ui->label->clear();
    ui->savedButton->setVisible(false);
}

    cv::Mat ErosionOperation:: applyErosion(const cv::Mat& inputImage) {
		ui->savedButton->setVisible(true);
        int erosionType = 0;
        if (ui->rectMorph->isChecked()) {
            erosionType = cv::MORPH_RECT;
        } else if (ui->ellipseMorph->isChecked()) {
            erosionType = cv::MORPH_ELLIPSE;
        } else if (ui->crossMorph->isChecked()) {
            erosionType = cv::MORPH_CROSS;
        }
        int kernelSize = ui->kernelSizeBox->value();
        int iteration = ui->iterationNumberBox->value();
        cv::Mat element = cv::getStructuringElement(erosionType, cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1), cv::Point(kernelSize, kernelSize));
        cv::Mat outputImage;
        cv::erode(inputImage, outputImage, element, cv::Point(-1, -1), iteration);

        return outputImage;
    }
