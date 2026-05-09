#include "DragAndDropWindow.h"
#include "ui_DragAndDropWindow.h"
#include "ErosionFilter.h"

#include <QDebug>
DragAndDropWindow::DragAndDropWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::Morphology) {
    ui->setupUi(this);
	erosionOp = new ErosionOperation(ui, this);
	ui->iterationNumberBox->setVisible(false);
	ui->iterationLabel->setVisible(false);
	ui->rectMorph->setVisible(false);
	ui->ellipseMorph->setVisible(false);
	ui->crossMorph->setVisible(false);
	ui->kernelSizeLabel->setVisible(false);
	ui->kernelSizeBox->setVisible(false);
	ui->resultLabel->setVisible(false);
	ui->erosionTypeLabel->setVisible(false);
	ui->ErosionFilter->setEnabled(false);
	ui->DilationFilter->setEnabled(false);
	ui->OpeningFilter->setEnabled(false);
	ui->ClosingFilter->setEnabled(false);
	connect(ui->label, &ImageDropLabel::imageDropped,
		this, [this](const QImage& img) {
			originalImage = img;
			ui->ErosionFilter->setEnabled(true);
			ui->DilationFilter->setEnabled(true);
			ui->OpeningFilter->setEnabled(true);
			ui->ClosingFilter->setEnabled(true);
		});
	ui->applyButton->setVisible(false);
	connect(ui->ErosionFilter, &QPushButton::clicked, this, &DragAndDropWindow::createErosionInterface);
	connect(ui->DilationFilter, &QPushButton::clicked, this, &DragAndDropWindow::createDilationInterface);
	connect(ui->OpeningFilter, &QPushButton::clicked, this, &DragAndDropWindow::createOpeningInterface);
	connect(ui->ClosingFilter, &QPushButton::clicked, this, &DragAndDropWindow::createClosingInterface);
	connect(ui->applyButton, &QPushButton::clicked, this, &DragAndDropWindow::applyCurrentFilter);

	

}

DragAndDropWindow::~DragAndDropWindow() {
    delete ui;
}
void DragAndDropWindow::createErosionInterface() {
	if (ui->DilationFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		erosionOp->showInterface();
	}
	else {
		erosionOp->hideInterface();
	}

	
}
void DragAndDropWindow::createDilationInterface() {
	if (ui->ErosionFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		ui->ErosionFilter->setVisible(false);
		ui->OpeningFilter->setVisible(false);
		ui->ClosingFilter->setVisible(false);
	}
	else {
		ui->ErosionFilter->setVisible(true);
		ui->OpeningFilter->setVisible(true);
		ui->ClosingFilter->setVisible(true);
	}
}
void DragAndDropWindow::createOpeningInterface() {
	if (ui->ErosionFilter->isVisible() || ui->DilationFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		ui->ErosionFilter->setVisible(false);
		ui->DilationFilter->setVisible(false);
		ui->ClosingFilter->setVisible(false);
	}
	else {
		ui->ErosionFilter->setVisible(true);
		ui->DilationFilter->setVisible(true);
		ui->ClosingFilter->setVisible(true);
	}
}
void DragAndDropWindow::createClosingInterface() {
	if (ui->ErosionFilter->isVisible() || ui->DilationFilter->isVisible() || ui->OpeningFilter->isVisible()) {
		ui->ErosionFilter->setVisible(false);
		ui->DilationFilter->setVisible(false);
		ui->OpeningFilter->setVisible(false);
	}
	else {
		ui->ErosionFilter->setVisible(true);
		ui->DilationFilter->setVisible(true);
		ui->OpeningFilter->setVisible(true);
	}

}
cv::Mat QImageToMat(const QImage& img) {
	QImage gray = img.convertToFormat(QImage::Format_Grayscale8);
	return cv::Mat(gray.height(), gray.width(), CV_8UC1,
		const_cast<uchar*>(gray.bits()),
		gray.bytesPerLine()).clone();
}

QImage MatToQImage(const cv::Mat& mat) {
	return QImage(mat.data, mat.cols, mat.rows,
		mat.step, QImage::Format_Grayscale8).copy();
}
void DragAndDropWindow::applyCurrentFilter() {
	if (originalImage.isNull()) return;

	cv::Mat input = QImageToMat(originalImage);
	cv::Mat output = erosionOp->applyErosion(input);
	QImage  result = MatToQImage(output);

	ui->resultLabel->move(700, 70);
	ui->resultLabel->resize(400, 381);
	ui->resultLabel->setVisible(true);
	ui->resultLabel->raise();          // ← ajoute ici
	ui->resultLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);  // ← ajoute ici

	QPixmap pix = QPixmap::fromImage(result).scaled(
		ui->resultLabel->size(),
		Qt::KeepAspectRatio,
		Qt::SmoothTransformation
	);

	ui->resultLabel->setPixmap(pix);
	ui->resultLabel->setStyleSheet("border: 2px dashed gray; background: transparent;");
	ui->resultLabel->raise();
	ui->resultLabel->repaint();
	ui->resultLabel->update();       // ← ajoute ici

	qDebug() << "hasPixmap:" << (!ui->resultLabel->pixmap().isNull());
}