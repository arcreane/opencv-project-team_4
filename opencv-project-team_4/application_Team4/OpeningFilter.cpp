#include "OpeningFilter.h"
#include "ui_DragAndDropWindow.h"
#include <opencv2/opencv.hpp>


OpeningOperation::OpeningOperation(Ui_Morphology* ui, QObject* parent)
    : QObject(parent), ui(ui) {
}

void OpeningOperation::showInterface() {
    ui->DilationFilter->setVisible(false);
    ui->ErosionFilter->setVisible(false);
    ui->ClosingFilter->setVisible(false);
    ui->OpeningFilter->move(20, 90);
    ui->label->move(280, ui->label->y());
    ui->label->resize(400, ui->label->height());
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
    ui->label->move(170, ui->label->y());
    ui->label->resize(431, ui->label->height());
    ui->DilationFilter->setVisible(true);
    ui->ErosionFilter->setVisible(true);
    ui->ClosingFilter->setVisible(true);
    ui->applyButton->setVisible(false);
	ui->OpeningFilter->move(700, 70);
}

cv::Mat OpeningOperation::applyOpening(const cv::Mat& inputImage) {

    int openingType = 0;
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
    cv::Mat element = cv::getStructuringElement(openingType, cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1), cv::Point(kernelSize, kernelSize));
    cv::Mat outputImage;
    cv::morphologyEx(inputImage, outputImage, cv::MORPH_OPEN, element, cv::Point(-1, -1), iteration);

    return outputImage;
}
