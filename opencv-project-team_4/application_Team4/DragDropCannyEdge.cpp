#include "DragDropCannyEdge.h"
#include "ui_DragDropCannyEdge.h"
#include "imagedropLabel.h"        
#include <opencv2/opencv.hpp>
#include <QDebug>
DragDropCannyEdge::DragDropCannyEdge(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::CannyEdge) {
	ui->setupUi(this);
	cannyOp = new CannyOperation(ui, this);
	resize(1000, 700);  // taille de départ raisonnable

	// Retirer le layout du .ui pour reprendre le contrôle
	delete centralWidget()->layout();

	repositionWidgets();
	ui->dragAndDrop->setStyleSheet("border: 2px inset gray;");
	ui->cannyFilterButton->setEnabled(false);
	ui->lowerTslider->setVisible(false);
	ui->higherTslider->setVisible(false);
	ui->lowerTlabel->setVisible(false);
	ui->higherTlabel->setVisible(false);
	ui->apertureBox->setVisible(false);
	ui->resultCannyLabel->setVisible(false);
	ui->apertureLabel->setVisible(false);
	ui->applyCannyFilterButton->setVisible(false);
	ui->dragAndDrop->setGrayscaleOnly(false);
	connect(ui->dragAndDrop, &ImageDropLabel::imageDropped,
		this, [this](const QImage& img) {
			originalImage = img;
			ui->cannyFilterButton->setEnabled(true);
			
		});
	connect(ui->cannyFilterButton, &QPushButton::clicked, this, &DragDropCannyEdge::showCannyInterface);
	connect(ui->applyCannyFilterButton, &QPushButton::clicked, this, &DragDropCannyEdge::onApplyCannyClicked);
	connect(ui->lowerTslider, &QSlider::valueChanged,this, &DragDropCannyEdge::onSliderChanged);
	connect(ui->higherTslider, &QSlider::valueChanged,this, &DragDropCannyEdge::onSliderChanged);
}

DragDropCannyEdge::~DragDropCannyEdge() {
	delete ui;
}
void DragDropCannyEdge::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);
	repositionWidgets();
}

void DragDropCannyEdge::repositionWidgets() {
	int Width = centralWidget()->width();
	int Height = centralWidget()->height();

	int marginX = Width * 0.04;
	int marginY = Height * 0.05;

	int titleH = Height * 0.07;

	int usableW = Width - 2 * marginX;
	int usableH = Height - titleH - marginY * 2;

	int column1 = usableW * 0.25;
	int column2 = usableW * 0.42;
	int column3 = usableW * 0.28;
	int gap = Width * 0.015;

	ui->titleLabelCanny->setGeometry(column2, marginY * 0.3, Width - 2 * marginX, titleH);
	ui->infoLabel->setGeometry(column2, marginY, Width - 2 * marginX, titleH);

	int c1 = marginX;
	int c2 = c1 + column1 + gap;
	int c3 = c2 + column2 + gap;

	int rowY = titleH + marginY;
	int dropH = usableH * 0.78;
	int btnH = usableH * 0.08;

	// Drop zone — centre
	ui->dragAndDrop->setGeometry(c2, rowY, column2, dropH);
	ui->resultCannyLabel->setGeometry(c3, rowY, column3+marginX, dropH - marginY);

	if (resizeBouton) {
		ui->cannyFilterButton->setGeometry(
			column2,
			rowY + dropH + gap,
			column1,
			btnH
		);
	}
	ui->applyCannyFilterButton->setGeometry(
		column2,
		rowY + dropH + gap,
		column1, btnH
	);
	//dynamique en fonction du bouton Apply
	if (!resizeBouton) {
		ui->cannyFilterButton->setGeometry(
			ui->applyCannyFilterButton->x() + ui->applyCannyFilterButton->width() + 10,
			ui->applyCannyFilterButton->y(),
			ui->cannyFilterButton->width(),
			ui->applyCannyFilterButton->height()
		);
	}
	int ctrlTop = rowY;
	int step = dropH / 7;

	ui->lowerTlabel->setGeometry(c1, ctrlTop + step * 0, column1, btnH);
	ui->lowerTslider->setGeometry(c1, ctrlTop + step * 1, column1, btnH);
	ui->higherTlabel->setGeometry(c1, ctrlTop + step * 2, column1, btnH);
	ui->higherTslider->setGeometry(c1, ctrlTop + step * 3, column1, btnH);
	ui->apertureLabel->setGeometry(c1, ctrlTop + step * 4, column1, btnH);
	ui->apertureBox->setGeometry(c1, ctrlTop + step * 5, column1, btnH);
}


void DragDropCannyEdge::showCannyInterface() {
	if (ui->cannyFilterButton->text() == "Canny Edge Filter") {
			cannyOp->showInterface();
			//Premiere Positionnement du bouton "Back" à droite du bouton "Apply"
			int x = ui->applyCannyFilterButton->x();
			int y = ui->applyCannyFilterButton->y();
			int h = ui->applyCannyFilterButton->height();
			int gap = 10;

			int labelW = ui->applyCannyFilterButton->width();
			int btnW = ui->cannyFilterButton->width();
			ui->cannyFilterButton->setGeometry(
				x + labelW + gap,
				y,
				btnW,
				h
			);
			resizeBouton = false;
	}
	else {
		cannyOp->hideInterface();
		resizeBouton = true;
		repositionWidgets();
	}
}


cv::Mat QImageToMatCanny(const QImage& img) {
	QImage gray = img.convertToFormat(QImage::Format_Grayscale8);
	return cv::Mat(gray.height(), gray.width(),CV_8UC1, const_cast<uchar*>(gray.bits()),
		gray.bytesPerLine()).clone();
}

QImage MatToQImageCanny(const cv::Mat& mat) {
	return QImage(mat.data, mat.cols, mat.rows,
		mat.step, QImage::Format_Grayscale8).copy();
}
void DragDropCannyEdge::onApplyCannyClicked() {
	if (originalImage.isNull()) return;
	cv::Mat output;
	cv::Mat input = QImageToMatCanny(originalImage);
	inputImage = input.clone();
	output = cannyOp->applyCannyEdgeFilter(input);
	cannyAppliedOnce = true;
	QImage  result = MatToQImageCanny(output);
	ui->resultCannyLabel->setVisible(true);
	ui->resultCannyLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	QPixmap pix = QPixmap::fromImage(result).scaled(
		ui->resultCannyLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	ui->resultCannyLabel->setPixmap(pix);
	ui->resultCannyLabel->setStyleSheet("border: 2px dashed gray; background: transparent;");
	ui->resultCannyLabel->raise();
	ui->resultCannyLabel->repaint();
	ui->resultCannyLabel->update();
}
void DragDropCannyEdge::onSliderChanged() {
	if (!cannyAppliedOnce) return;
	onApplyCannyClicked();
}