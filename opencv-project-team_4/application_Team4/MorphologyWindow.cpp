#include "MorphologyWindow.h"
#include "ui_MorphologyWindow.h"
#include "ErosionFilter.h"
#include "DilationFilter.h"
#include <QFileDialog>
#include "StartInterface.h"

#include <QDebug>
MorphologyWindow::MorphologyWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::Morphology) {
	//setting the starting interface for the morphology operations, with the image drop area and filter selection buttons.
	ui->setupUi(this);
	applyStyles();
	resize(1000, 700);  
	delete centralWidget()->layout();

	repositionWidgets();
	//defintion of the possible available buttons and picking the operarion
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
	ui->savedButton->setVisible(false);
	ui->label->setGrayscaleOnly(true);
	ui->LabelTitle->setAlignment(Qt::AlignCenter);
	//use of the ImageDropLabel class to allow users to drag and drop an image into the application, enabling the filter options once an image is loaded.
	connect(ui->label, &ImageDropLabel::imageDropped,this, [this](const QImage& img) {originalImage = img;
			ui->ErosionFilter->setEnabled(true);
			ui->DilationFilter->setEnabled(true);
			ui->OpeningFilter->setEnabled(true);
			ui->ClosingFilter->setEnabled(true);
		});
	ui->applyButton->setVisible(false);
	//definition of the tasks
	connect(ui->ErosionFilter, &QPushButton::clicked, this, &MorphologyWindow::createErosionInterface);
	connect(ui->DilationFilter, &QPushButton::clicked, this, &MorphologyWindow::createDilationInterface);
	connect(ui->OpeningFilter, &QPushButton::clicked, this, &MorphologyWindow::createOpeningInterface);
	connect(ui->ClosingFilter, &QPushButton::clicked, this, &MorphologyWindow::createClosingInterface);
	connect(ui->applyButton, &QPushButton::clicked, this, &MorphologyWindow::applyCurrentFilter);
	connect(ui->savedButton, &QPushButton::clicked, this, &MorphologyWindow::saveImage);
	connect(ui->BackButton, &QPushButton::clicked, this, &MorphologyWindow::backToStartInterface);

}

void MorphologyWindow::backToStartInterface() {
	this->close();
	StartInterface* start = new StartInterface(this);
	start->show();
}

//Function to save the image in the PC after the transformation, not possible to save the original image
void MorphologyWindow::saveImage() {
	QPixmap pix = ui->resultLabel->pixmap(Qt::ReturnByValue);
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
void MorphologyWindow::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);
	repositionWidgets();
}
//Function to make it responsive and try to adapt it to the resize of the window, with a dynamic repositioning of the widgets based on the current size of the window,
// using proportions to maintain a consistent layout.
void MorphologyWindow::repositionWidgets() {
	//basic proportion
	int Width = this->width();
	int Height = this->height();

	int marginX = Width * 0.08;  
	int marginY = Height * 0.08; 

	//title
	int titleH = Height * 0.06;
	ui->LabelTitle->setGeometry(marginX, Height * 0.01, Width - 2 * marginX, titleH);
	ui->LabelTitle->setAlignment(Qt::AlignCenter);

	int usableW = Width - 2 * marginX;
	int usableH = Height - 2 * marginY;
	// 3 columns : 30% | 40% | 30%
	int column1 = usableW * 0.30;
	int column2 = usableW * 0.40;
	int column3 = usableW * 0.30;
	int gap = Width * 0.01;

	int c1 = marginX;
	int c2 = c1 + column1 + gap;
	int c3 = c2 + column2 + gap;

	int dropH = usableH * 0.80;
	int btnH = usableH * 0.07;
	int rowY = marginY;
	//defintion of the output label and the button to save and to apply the modification, with a dynamic positioning based on the current size of the window.
	ui->label->setGeometry(c2, rowY, column2, dropH);
	ui->resultLabel->setGeometry(c3, rowY, column3, dropH);
	ui->applyButton->setGeometry(
		c2 + (column2 - column1) / 2,
		rowY + dropH + gap * 2,
		column1, btnH
	);
	ui->savedButton->setGeometry(
		c3 + (column3 - column1) / 2,
		rowY + dropH + gap * 2,
		column1, btnH
	);
	//SetGeometry(x, y, width, height) : Button
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
	ui->BackButton->setGeometry(0, 0, 200, 40);
	ui->BackButton->setStyleSheet("font-family: 'Segoe UI'; font-size: 14px; font-weight: bold; color: #d0d0ff; background-color: #1a1a2e; border: 2px solid #d0d0ff; border-radius: 5px;");
}

MorphologyWindow::~MorphologyWindow() {
    delete ui;
}

//Methods to access the right interfaces
void MorphologyWindow::createErosionInterface() {
	if (ui->DilationFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		erosionOp->showInterface();
	}
	else {
		erosionOp->hideInterface();
	}
	repositionWidgets();
	
}
void MorphologyWindow::createDilationInterface() {
	if (ui->ErosionFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		ui->DilationFilter->setGeometry(ui->ErosionFilter->geometry());
		dilationOp->showInterface();
	}
	else {
		dilationOp->hideInterface();
		repositionWidgets();
	}

}
void MorphologyWindow::createOpeningInterface() {
	if (ui->ErosionFilter->isVisible() || ui->DilationFilter->isVisible() || ui->ClosingFilter->isVisible()) {
		ui->OpeningFilter->setGeometry(ui->ErosionFilter->geometry());
		openingOp->showInterface();
	}
	else {
		openingOp->hideInterface();
		repositionWidgets();

	}

}
void MorphologyWindow::createClosingInterface() {
	if (ui->ErosionFilter->isVisible() || ui->OpeningFilter->isVisible() || ui->DilationFilter->isVisible()) {
		ui->ClosingFilter->setGeometry(ui->ErosionFilter->geometry());
		closingOp->showInterface();
	}
	else {
		closingOp->hideInterface();
		repositionWidgets();
	}
}

//function  to convert the QImage into a cv::Mat in order to well use the OpenCV library
cv::Mat QImageToMat(const QImage& img) {
	QImage gray = img.convertToFormat(QImage::Format_Grayscale8);
	return cv::Mat(gray.height(), gray.width(), CV_8UC1,
		const_cast<uchar*>(gray.bits()),
		gray.bytesPerLine()).clone();
}
//Conversion from OpenCV to QtDesigner for the resultant image
QImage MatToQImage(const cv::Mat& mat) {
	return QImage(mat.data, mat.cols, mat.rows,
		mat.step, QImage::Format_Grayscale8).copy();
}

//Application of the different possible filters by using the method define in each specific class,
// and displaying the result in the right part of the window, with a dynamic resizing to fit the label.
void MorphologyWindow::applyCurrentFilter() {
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

//CSS
void MorphologyWindow::applyStyles() {
	setStyleSheet(R"(
QMainWindow { background-color: #0d0d1c; }
QWidget#centralwidget { background-color: #0d0d1c; }

QLabel#LabelTitle {
    color: #d0d0ff; font-family: 'Segoe UI'; font-size: 20px;
    font-weight: bold; letter-spacing: 3px; background: transparent;
}
QLabel#label {
    background-color: #07070f; border: 2px dashed #252558;
    border-radius: 10px; color: #40406a;
    font-family: 'Segoe UI'; font-size: 16px;
}
QLabel#label:hover { border-color: #4848a8; background-color: #0a0a18; }

QLabel#resultLabel {
    background-color: #07070f; border: 1px solid #1a1a38;
    border-radius: 10px; color: #30304c;
    font-family: 'Segoe UI'; font-size: 14px;
}

QLabel#kernelSizeLabel, QLabel#iterationLabel,
QLabel#erosionTypeLabel {
    color: #8888aa; font-family: 'Segoe UI';
    font-size: 16px; background: transparent;
}

QSpinBox#kernelSizeBox, QSpinBox#iterationNumberBox {
    background-color: #0a0a1c; border: 1px solid #252550;
    border-radius: 6px; color: #c0c0e0;
    padding: 3px 8px; font-family: 'Segoe UI'; font-size: 12px;
}
QSpinBox#kernelSizeBox:hover, QSpinBox#iterationNumberBox:hover {
    border-color: #4848a8;
}

QRadioButton#rectMorph, QRadioButton#ellipseMorph, QRadioButton#crossMorph {
    color: #a0a0c8; font-family: 'Segoe UI'; font-size: 16px;
    background: transparent;
}
QRadioButton::indicator {
    width: 14px; height: 14px; border-radius: 7px;
    border: 2px solid #252558; background: #07070f;
}
QRadioButton::indicator:checked { background: #5050c8; border-color: #8080e8; }

QPushButton {
    background-color: #16162e; color: #b0b0d0;
    border: 1px solid #26264c; border-radius: 8px;
    padding: 6px 14px; font-family: 'Segoe UI';
    font-size: 12px; font-weight: 500;
}
QPushButton:hover { background-color: #20204a; border-color: #3c3c8a; color: #dcdcff; }
QPushButton:pressed { background-color: #0c0c20; border-color: #5858b0; }
QPushButton:disabled { background-color: #0c0c18; border-color: #141428; color: #303050; }

QPushButton#ErosionFilter, QPushButton#DilationFilter, QPushButton#OpeningFilter, QPushButton#ClosingFilter {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff; border: none; border-radius: 10px;
    font-size: 17px; font-weight: bold;
}
QPushButton#ErosionFilter:hover, QPushButton#DilationFilter:hover, QPushButton#OpeningFilter:hover, QPushButton#ClosingFilter:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#ErosionFilter:disabled, QPushButton#DilationFilter:disabled, QPushButton#OpeningFilter:disabled,QPushButton#ClosingFilter:disabled {
    background: #141428; color: #303050;
}

QPushButton#applyButton {
    background-color: #0a2518; border-color: #185a35;
    color: #48c880; border-radius: 10px;
    font-size: 13px; font-weight: bold;
}
QPushButton#applyButton:hover {
    background-color: #103020; border-color: #287a50; color: #68e8a0;
}

QPushButton#savedButton {
    background-color: #0a1828; border-color: #183868;
    color: #4898d0; border-radius: 10px;
    font-size: 13px; font-weight: bold;
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