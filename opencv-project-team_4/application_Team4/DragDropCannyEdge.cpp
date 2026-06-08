#include "DragDropCannyEdge.h"
#include "ui_DragDropCannyEdge.h"
#include "imagedropLabel.h"        
#include "StartInterface.h"
#include <opencv2/opencv.hpp>
#include <QFileDialog>
#include <QDebug>
DragDropCannyEdge::DragDropCannyEdge(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::CannyEdge) {
	//preparing the interface with the right dimension and style
	ui->setupUi(this);
	applyStyles();
	cannyOp = new CannyOperation(ui, this);
	resize(1000, 700);  
	delete centralWidget()->layout();
	repositionWidgets();

	//loading the initial interface with the right settings and connections
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

	//use of the class ImageDropLabel to handle the drag and drop of images and connect it to the right slots to update the interface accordingly
	connect(ui->dragAndDrop, &ImageDropLabel::imageDropped,this, [this](const QImage& img) {originalImage = img;ui->cannyFilterButton->setEnabled(true);});
	connect(ui->cannyFilterButton, &QPushButton::clicked, this, &DragDropCannyEdge::showCannyInterface);
	connect(ui->applyCannyFilterButton, &QPushButton::clicked, this, &DragDropCannyEdge::onApplyCannyClicked);
	connect(ui->lowerTslider, &QSlider::valueChanged,this, &DragDropCannyEdge::onSliderChanged);
	connect(ui->higherTslider, &QSlider::valueChanged,this, &DragDropCannyEdge::onSliderChanged);
	connect(ui->savedButton, &QPushButton::clicked, this, &DragDropCannyEdge::saveImage);
	connect(ui->BackButton, &QPushButton::clicked, this, &DragDropCannyEdge::backToStartInterface);
}

//Function to return to the start interface when the back button is clicked, closing the current interface and opening a new instance of the start interface.
void DragDropCannyEdge::backToStartInterface() {
	this->close();
	StartInterface* start = new StartInterface();
	start->show();
}
DragDropCannyEdge::~DragDropCannyEdge() {
	delete ui;
}
void DragDropCannyEdge::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);
	repositionWidgets();
}


//Function to reposition the widgets in the interface based on the current size of the window, ensuring a responsive design that adapts to different window sizes while maintaining a consistent layout and spacing between elements.
void DragDropCannyEdge::repositionWidgets() {
	//base elements
	int Width = this->width();
	int Height = this->height();

	if (Width < 200 || Height < 200) return;

	int marginX = Width * 0.04;
	int marginY = Height * 0.05;

	//title
	int titleH = Height * 0.07;
	int usableW = Width - 2 * marginX;
	int usableH = Height - titleH - marginY * 2;

	//split the interface into columns and rows
	int column1 = usableW * 0.25;
	int column2 = usableW * 0.42;
	int column3 = usableW * 0.28;
	int gap = Width * 0.015;

	//calculate the positions of the columns and rows
	int c1 = marginX;
	int c2 = c1 + column1 + gap;
	int c3 = c2 + column2 + gap;

	int rowY = titleH + marginY;
	int dropH = usableH * 0.78;
	int btnH = usableH * 0.08;

	// Title + lables centred
	ui->titleLabelCanny->setGeometry(c2 + marginX, marginY * 0.3, usableW, titleH);
	ui->titleLabelCanny->setAlignment(Qt::AlignCenter);
	ui->infoLabel->setGeometry(0, marginY * 0.3 + titleH, usableW, titleH * 0.6);
	ui->infoLabel->setAlignment(Qt::AlignCenter);

	// Drop zone et result
	ui->dragAndDrop->setGeometry(c2, rowY, column2, dropH);
	ui->resultCannyLabel->setGeometry(c3, rowY, column3, dropH);

	// 3 buttons same size at the bottom
	int totalBtnW = c3 + column3 - c1;  
	int btnW = (totalBtnW - 2 * gap) / 3;
	int btnY = rowY + dropH + gap;

	// Adjust button visibility and positions based on the resizeBouton flag, in order to follow the same structure of the interface when the canny filter options are shown or hidden
	//ANd coherence with the morphology window
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

	//position of the parameters
	int step = dropH / 7;
	ui->lowerTlabel->setGeometry(c1, rowY + step * 0, column1, btnH);
	ui->lowerTslider->setGeometry(c1, rowY + step * 1, column1, btnH);
	ui->higherTlabel->setGeometry(c1, rowY + step * 2, column1, btnH);
	ui->higherTslider->setGeometry(c1, rowY + step * 3, column1, btnH);
	ui->apertureLabel->setGeometry(c1, rowY + step * 4, column1, btnH);
	ui->apertureBox->setGeometry(c1, rowY + step * 5, column1, btnH);
	ui->BackButton->setGeometry(0, 0, 200, 40);
	ui->BackButton->setStyleSheet("font-family: 'Segoe UI'; font-size: 14px; font-weight: bold; color: #d0d0ff; background-color: #1a1a2e; border: 2px solid #d0d0ff; border-radius: 5px;");
}

//go to Canny interface when the button is clicked, showing the parameters and the apply button, and hiding the canny filter button, and repositioning the widgets accordingly to maintain a coherent layout.
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

//Convert the input image from QImage to cv::Mat format, ensuring that the image is in grayscale format (CV_8UC1) which is required for the Canny edge detection algorithm, and returning a clone of the resulting cv::Mat to ensure that the data is properly managed and can be safely modified without affecting the original QImage.
cv::Mat QImageToMatCanny(const QImage& img) {
	QImage gray = img.convertToFormat(QImage::Format_Grayscale8);
	return cv::Mat(gray.height(), gray.width(),CV_8UC1, const_cast<uchar*>(gray.bits()),
		gray.bytesPerLine()).clone();
}

//Convert the input image from cv::Mat to QImage format, ensuring that the image is in grayscale format (QImage::Format_Grayscale8)
// which is required for displaying the Canny edge detection results, and returning a copy of the resulting
// QImage to ensure that the data is properly managed and can be safely modified without affecting the original cv::Mat.
QImage MatToQImageCanny(const cv::Mat& mat) {
	return QImage(mat.data, mat.cols, mat.rows,
		mat.step, QImage::Format_Grayscale8).copy();
}

//Apply the canny edge detection Algo
void DragDropCannyEdge::onApplyCannyClicked() {
	if (originalImage.isNull()) return;
	cv::Mat output;
	//conversion
	cv::Mat input = QImageToMatCanny(originalImage);
	inputImage = input.clone();

	//use of the canny operation defined in CannyFilter.cpp to apply the canny edge detection algorithm to the input image, and storing the result in the output cv::Mat.
	output = cannyOp->applyCannyEdgeFilter(input);
	cannyAppliedOnce = true; // to make more easier the update of the image with the canging of the parameters
	QImage  result = MatToQImageCanny(output);
	ui->resultCannyLabel->setVisible(true);
	ui->resultCannyLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	//show the results
	QPixmap pix = QPixmap::fromImage(result).scaled(
		ui->resultCannyLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	ui->resultCannyLabel->setPixmap(pix);
	ui->resultCannyLabel->setStyleSheet("border: 2px dashed gray; background: transparent;");
	ui->resultCannyLabel->raise();
	ui->resultCannyLabel->repaint();
	ui->resultCannyLabel->update();
}

//make the slider dynamics if canny edge has been applied once
void DragDropCannyEdge::onSliderChanged() {
	if (!cannyAppliedOnce) return;
	onApplyCannyClicked();
}

//CSS
void DragDropCannyEdge::applyStyles() {
	setStyleSheet(R"(
QMainWindow { background-color: #0d0d1c; }
QWidget#centralwidget { background-color: #0d0d1c; }

QLabel#titleLabelCanny {
    color: #d0d0ff; font-family: 'Segoe UI'; font-size: 20px;
    font-weight: bold; letter-spacing: 3px; background: transparent;
}
QLabel#infoLabel {
    color: #50508a; font-family: 'Segoe UI'; font-size: 14px;
    background: transparent;
}
QLabel#dragAndDrop {
    background-color: #07070f; border: 2px dashed #252558;
    border-radius: 10px; color: #40406a;
    font-family: 'Segoe UI'; font-size: 13px;
}
QLabel#dragAndDrop:hover { border-color: #4848a8; background-color: #0a0a18; }

QLabel#resultCannyLabel {
    background-color: #07070f; border: 1px solid #1a1a38;
    border-radius: 10px; color: #30304c;
    font-family: 'Segoe UI'; font-size: 14px;
}

QSlider::groove:horizontal {
    background: #1a1a38; height: 4px; border-radius: 2px;
}
QSlider::handle:horizontal {
    background: #5050c8; width: 14px; height: 14px;
    margin: -5px 0; border-radius: 7px;
}
QSlider::handle:horizontal:hover { background: #7878e8; }
QSlider::sub-page:horizontal { background: #3838a8; border-radius: 2px; }

QLabel#lowerTlabel, QLabel#higherTlabel, QLabel#apertureLabel {
    color: #8888aa; font-family: 'Segoe UI'; font-size: 14px;
    background: transparent;
}

QComboBox#apertureBox {
    background-color: #0a0a1c; border: 1px solid #252550;
    border-radius: 8px; color: #c0c0e0;
    padding: 4px 10px; font-family: 'Segoe UI'; font-size: 12px;
}
QComboBox#apertureBox:hover { border-color: #4848a8; background-color: #0e0e26; }
QComboBox QAbstractItemView {
    background-color: #0d0d1c; border: 1px solid #252550;
    color: #c0c0e0; selection-background-color: #28286a;
}

QPushButton {
    background-color: #16162e; color: #b0b0d0;
    border: 1px solid #26264c; border-radius: 8px;
    padding: 6px 14px; font-family: 'Segoe UI';
    font-size: 12px; font-weight: 500;
}
QPushButton:hover { background-color: #20204a; border-color: #3c3c8a; color: #dcdcff; }
QPushButton:pressed { background-color: #0c0c20; border-color: #5858b0; }
QPushButton:disabled { background-color: #0c0c18; border-color: #141428; color: #303050; }

QPushButton#cannyFilterButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff; border: none; border-radius: 10px;
    font-size: 14px; font-weight: bold; letter-spacing: 1px;
}
QPushButton#cannyFilterButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#cannyFilterButton:disabled { background: #141428; color: #303050; }

QPushButton#applyCannyFilterButton {
    background-color: #0a2518; border-color: #185a35;
    color: #48c880; border-radius: 10px;
    font-size: 14px; font-weight: bold;
}
QPushButton#applyCannyFilterButton:hover {
    background-color: #103020; border-color: #287a50; color: #68e8a0;
}

QPushButton#savedButton {
    background-color: #0a1828; border-color: #183868;
    color: #4898d0; border-radius: 10px;
    font-size: 14px; font-weight: bold;
}
QPushButton#savedButton:hover {
    background-color: #101e32; border-color: #285898; color: #68b8f0;
}

QScrollBar:vertical { background: #07070f; width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: #252558; border-radius: 3px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background: #3838a0; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: #07070f; height: 6px; border-radius: 3px; }
QScrollBar::handle:horizontal { background: #252558; border-radius: 3px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)");
}