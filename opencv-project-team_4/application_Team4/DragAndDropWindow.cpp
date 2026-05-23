#include "DragAndDropWindow.h"
#include "ui_DragAndDropWindow.h"
#include "ErosionFilter.h"
#include "DilationFilter.h"

#include <QDebug>
DragAndDropWindow::DragAndDropWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::Morphology) {

	ui->setupUi(this);
	resize(1000, 700);  // taille de départ raisonnable

	// Retirer le layout du .ui pour reprendre le contrôle
	delete centralWidget()->layout();

	repositionWidgets();
	erosionOp = new ErosionOperation(ui, this);
	dilationOp = new DilationOperation(ui, this);
	openingOp = new OpeningOperation(ui, this);
	closingOp = new ClosingOperation(ui, this);
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
	ui->label->setGrayscaleOnly(true);
	ui->LabelTitle->setAlignment(Qt::AlignCenter);

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

void DragAndDropWindow::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);
	repositionWidgets();
}

void DragAndDropWindow::repositionWidgets() {
	int Width = this->width();
	int Height = this->height();

	int marginX = Width * 0.08;   // marge horizontale
	int marginY = Height * 0.08;   // marge verticale haute

	int titleH = Height * 0.06;
	ui->LabelTitle->setGeometry(marginX, Height * 0.01, Width - 2 * marginX, titleH);
	ui->LabelTitle->setAlignment(Qt::AlignCenter);

	int usableW = Width - 2 * marginX;
	int usableH = Height - 2 * marginY;
	// 3 colonnes : 30% | 40% | 30%
	int column1 = usableW * 0.30;
	int column2 = usableW * 0.40;
	int column3 = usableW * 0.30;
	int gap = Width * 0.01;

	// Positions horizontales des colonnes
	int c1 = marginX;
	int c2 = c1 + column1 + gap;
	int c3 = c2 + column2 + gap;

	int dropH = usableH * 0.80;
	int btnH = usableH * 0.07;
	int rowY = marginY;

	ui->label->setGeometry(c2, rowY, column2, dropH);
	ui->resultLabel->setGeometry(c3, rowY, column3, dropH);
	ui->applyButton->setGeometry(
		c2 + (column2 - column1) / 2,
		rowY + dropH + gap * 2,
		column1, btnH
	);
	//SetGeometry(x, y, width, height)
	ui->ErosionFilter->setGeometry(c1, rowY, column1, btnH);
	ui->DilationFilter->setGeometry(c1, rowY + dropH - btnH, column1, btnH);
	ui->OpeningFilter->setGeometry(c3, rowY, column3, btnH);
	ui->ClosingFilter->setGeometry(c3, rowY + dropH - btnH, column3, btnH);
	int ctrlTop = rowY + btnH + gap * 2;
	int ctrlH = dropH - 2 * (btnH + gap * 2);
	int step = ctrlH / 8;
	ui->kernelSizeLabel->setGeometry(c1, ctrlTop + step * 0, column1, btnH);
	ui->kernelSizeBox->setGeometry(c1, ctrlTop + step * 1, column1, btnH);
	ui->iterationLabel->setGeometry(c1, ctrlTop + step * 2, column1, btnH);
	ui->iterationNumberBox->setGeometry(c1, ctrlTop + step * 3, column1, btnH);
	ui->erosionTypeLabel->setGeometry(c1, ctrlTop + step * 4, column1, btnH);
	ui->rectMorph->setGeometry(c1, ctrlTop + step * 5, column1, btnH);
	ui->ellipseMorph->setGeometry(c1, ctrlTop + step * 6, column1, btnH);
	ui->crossMorph->setGeometry(c1, ctrlTop + step * 7, column1, btnH);
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
	repositionWidgets();
	
}
void DragAndDropWindow::createDilationInterface() {
	if (ui->ErosionFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		ui->DilationFilter->setGeometry(ui->ErosionFilter->geometry());
		dilationOp->showInterface();
	}
	else {
		dilationOp->hideInterface();
		repositionWidgets();
	}

}
void DragAndDropWindow::createOpeningInterface() {
	if (ui->ErosionFilter->isVisible() || ui->DilationFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		ui->OpeningFilter->setGeometry(ui->ErosionFilter->geometry());
		openingOp->showInterface();
	}
	else {
		openingOp->hideInterface();
		repositionWidgets();

	}

}
void DragAndDropWindow::createClosingInterface() {
	if (ui->ErosionFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->DilationFilter->isVisible()) {
		ui->ClosingFilter->setGeometry(ui->ErosionFilter->geometry());
		closingOp->showInterface();
	}
	else {
		closingOp->hideInterface();
		repositionWidgets();
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
	cv::Mat output;
	cv::Mat input = QImageToMat(originalImage);

	if (ui->ErosionFilter->isVisible())  output = erosionOp->applyErosion(input);
	if (ui->DilationFilter->isVisible()) output = dilationOp->applyDilation(input);
	if (ui->OpeningFilter->isVisible())  output = openingOp->applyOpening(input);
	if (ui->ClosingFilter->isVisible())  output = closingOp->applyClosing(input);

	QImage result = MatToQImage(output);
	ui->resultLabel->setVisible(true);
	ui->resultLabel->setAlignment(Qt::AlignCenter);
	ui->resultLabel->setStyleSheet("border: 2px dashed gray; background: transparent;");

	QPixmap pix = QPixmap::fromImage(result).scaled(
		ui->resultLabel->size(),
		Qt::KeepAspectRatio,
		Qt::SmoothTransformation
	);
	ui->resultLabel->setPixmap(pix);
}