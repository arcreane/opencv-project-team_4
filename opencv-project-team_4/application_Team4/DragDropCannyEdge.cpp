#include "DragDropCannyEdge.h"
#include "ui_DragDropCannyEdge.h"
#include "imagedropLabel.h"        
#include <opencv2/opencv.hpp>
#include <QDebug>
DragDropCannyEdge::DragDropCannyEdge(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::CannyEdge) {
	ui->setupUi(this);
	cannyOp = new CannyOperation(ui, this);

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
}

DragDropCannyEdge::~DragDropCannyEdge() {
	delete ui;
}

void DragDropCannyEdge::showCannyInterface() {
	cannyOp->showInterface();	
}
void DragDropCannyEdge::hideCannyInterface() {
	cannyOp->hideInterface();
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
	output = cannyOp->applyCannyEdgeFilter(input);
	QImage  result = MatToQImageCanny(output);
	ui->resultCannyLabel->move(700, 70);
	ui->resultCannyLabel->resize(400, 381);
	ui->resultCannyLabel->setVisible(true);
	ui->resultCannyLabel->raise();
	ui->resultCannyLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	QPixmap pix = QPixmap::fromImage(result).scaled(
		ui->resultCannyLabel->size(),
		Qt::KeepAspectRatio,
		Qt::SmoothTransformation
	);

	ui->resultCannyLabel->setPixmap(pix);
	ui->resultCannyLabel->setStyleSheet("border: 2px dashed gray; background: transparent;");
	ui->resultCannyLabel->raise();
	ui->resultCannyLabel->repaint();
	ui->resultCannyLabel->update();
}
