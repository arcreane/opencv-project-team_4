#include "DragDropCannyEdge.h"
#include "ui_DragDropCannyEdge.h"
#include "imagedropLabel.h"        
#include <opencv2/opencv.hpp>
#include <QFileDialog>
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
	ui->savedButton->setVisible(false);
	connect(ui->dragAndDrop, &ImageDropLabel::imageDropped,
		this, [this](const QImage& img) {
			originalImage = img;
			ui->cannyFilterButton->setEnabled(true);
			
		});
	connect(ui->cannyFilterButton, &QPushButton::clicked, this, &DragDropCannyEdge::showCannyInterface);
	connect(ui->applyCannyFilterButton, &QPushButton::clicked, this, &DragDropCannyEdge::onApplyCannyClicked);
	connect(ui->lowerTslider, &QSlider::valueChanged,this, &DragDropCannyEdge::onSliderChanged);
	connect(ui->higherTslider, &QSlider::valueChanged,this, &DragDropCannyEdge::onSliderChanged);
	connect(ui->savedButton, &QPushButton::clicked, this, &DragDropCannyEdge::saveImage);
}

DragDropCannyEdge::~DragDropCannyEdge() {
	delete ui;
}
void DragDropCannyEdge::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);
	repositionWidgets();
}

void DragDropCannyEdge::repositionWidgets() {
	int Width = this->width();
	int Height = this->height();

	if (Width < 200 || Height < 200) return;

	int marginX = Width * 0.04;
	int marginY = Height * 0.05;

	int titleH = Height * 0.07;
	int usableW = Width - 2 * marginX;
	int usableH = Height - titleH - marginY * 2;

	int column1 = usableW * 0.25;
	int column2 = usableW * 0.42;
	int column3 = usableW * 0.28;
	int gap = Width * 0.015;

	int c1 = marginX;
	int c2 = c1 + column1 + gap;
	int c3 = c2 + column2 + gap;

	int rowY = titleH + marginY;
	int dropH = usableH * 0.78;
	int btnH = usableH * 0.08;

	// Titre centré
	ui->titleLabelCanny->setGeometry(c2 + marginX, marginY * 0.3, usableW, titleH);
	ui->titleLabelCanny->setAlignment(Qt::AlignCenter);
	ui->infoLabel->setGeometry(0, marginY * 0.3 + titleH, usableW, titleH * 0.6);
	ui->infoLabel->setAlignment(Qt::AlignCenter);

	// Drop zone et result
	ui->dragAndDrop->setGeometry(c2, rowY, column2, dropH);
	ui->resultCannyLabel->setGeometry(c3, rowY, column3, dropH);

	// 3 boutons même taille en bas
	int totalBtnW = c3 + column3 - c1;  
	int btnW = (totalBtnW - 2 * gap) / 3;
	int btnY = rowY + dropH + gap;

	if (resizeBouton) {
		ui->cannyFilterButton->setGeometry(c1 + btnW + gap, btnY, btnW, btnH);
		ui->applyCannyFilterButton->setVisible(false);
		ui->savedButton->setVisible(false);
	}
	else {
		ui->applyCannyFilterButton->setGeometry(c1, btnY, btnW, btnH);
		ui->cannyFilterButton->setGeometry(c1 + btnW + gap, btnY, btnW, btnH);
		ui->savedButton->setGeometry(c1 + (btnW + gap) * 2, btnY, btnW, btnH);
	}

	int step = dropH / 7;
	ui->lowerTlabel->setGeometry(c1, rowY + step * 0, column1, btnH);
	ui->lowerTslider->setGeometry(c1, rowY + step * 1, column1, btnH);
	ui->higherTlabel->setGeometry(c1, rowY + step * 2, column1, btnH);
	ui->higherTslider->setGeometry(c1, rowY + step * 3, column1, btnH);
	ui->apertureLabel->setGeometry(c1, rowY + step * 4, column1, btnH);
	ui->apertureBox->setGeometry(c1, rowY + step * 5, column1, btnH);
}


void DragDropCannyEdge::showCannyInterface() {
	if (ui->cannyFilterButton->text() == "Canny Edge Filter") {
			cannyOp->showInterface();
			resizeBouton = false;
			repositionWidgets();
	}
	else {
		cannyOp->hideInterface();
		resizeBouton = true;
		repositionWidgets();
	}
}
void DragDropCannyEdge::saveImage() {
	QPixmap pix = ui->resultCannyLabel->pixmap(Qt::ReturnByValue);
	if (!pix.isNull()) {
		QString fileName = QFileDialog::getSaveFileName(
			this, "Save Image", "",
			"PNG Image (*.png);;JPEG Image (*.jpg)"
		);
		if (!fileName.isEmpty()) {
			pix.save(fileName);
		}
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