#include "StartInterface.h"
#include "ui_StartInterface.h"
#include "MorphologyWindow.h"
#include "DragDropCannyEdge.h"
#include "DeepLearningWindow.h"
#include "HistogramEqualization.h"




#include <QDebug>
StartInterface::StartInterface(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::StartInterface) {
	ui->setupUi(this);
	applyStyles();  
	connect(ui->CannyEdgeButton, &QPushButton::clicked, this, &StartInterface::goToCannyEdgeInterface);
	connect(ui->GeometricTransfButton, &QPushButton::clicked, this, &StartInterface::goToGeometricInterface);
	connect(ui->HistogramButton, &QPushButton::clicked, this, &StartInterface::goToHistogramInterface);
	connect(ui->MorphologyButton, &QPushButton::clicked, this, &StartInterface::goToMorphologyInterface);
	connect(ui->PanoramaButton, &QPushButton::clicked, this, &StartInterface::goToPanoramaInterface);
	connect(ui->ThresholdingButton, &QPushButton::clicked, this, &StartInterface::goToThresholdingInterface);
	connect(ui->VideoButton, &QPushButton::clicked, this, &StartInterface::goToVideoInterface);
	connect(ui->DeepButton, &QPushButton::clicked, this, &StartInterface::goToDeepLearningInterface);
	connect(ui->InteractiveButton, &QPushButton::clicked, this, &StartInterface::goToInteractiveInterface);
	connect(ui->CreativeEffectButton, &QPushButton::clicked, this, &StartInterface::goToCreativeInterface);
}

StartInterface::~StartInterface() {
	delete ui;
}
void StartInterface::goToMorphologyInterface() {
	MorphologyWindow* morph = new MorphologyWindow(nullptr);
	morph->show();
	hide();
}
void StartInterface::goToThresholdingInterface() {
	// Implement the logic to navigate to the Thresholding Interface

}
void StartInterface::goToCannyEdgeInterface() {
	DragDropCannyEdge* canny = new DragDropCannyEdge(nullptr);
	canny->show();
	hide();
}
void StartInterface::goToGeometricInterface() {
	GeometricTransformWindow* geo = new GeometricTransformWindow(this, nullptr);
	geo->show();
	hide();
}
void StartInterface::goToPanoramaInterface() {
	PanoramaStitching* panorama = new PanoramaStitching(this, nullptr);
	panorama->show();
	hide();
}
void StartInterface::goToHistogramInterface() {
	HistogramEqualizationWindow* hist = new HistogramEqualizationWindow(this);
	hist->show();
	hide();
}
void StartInterface::goToVideoInterface() {
	VideoProcessingWindow* video = new VideoProcessingWindow(this, nullptr);
	video->show();
	hide();
}
void StartInterface::goToDeepLearningInterface() {
	DeepLearningWindow* deep = new DeepLearningWindow(nullptr);
	deep->show();
	hide();
}
void StartInterface::goToInteractiveInterface() {
	// Implement the logic to navigate to the Interactive Interface

}
void StartInterface::goToCreativeInterface() {
	// Implement the logic to navigate to the Creative Interface

}
void StartInterface::applyStyles() {
    setStyleSheet(R"(
QMainWindow { background-color: #0d0d1c; }
QWidget#centralwidget { background-color: #0d0d1c; }

QLabel {
    color: #c0c0e0;
    font-family: 'Segoe UI';
    font-size: 18px;
    background: transparent;
}

QPushButton {
    background-color: #16162e;
    color: #b0b0d0;
    border: 1px solid #26264c;
    border-radius: 8px;
    padding: 12px 20px;
    font-family: 'Segoe UI';
    font-size: 13px;
    font-weight: 500;
}
QPushButton:hover {
    background-color: #20204a;
    border-color: #3c3c8a;
    color: #dcdcff;
}
QPushButton:pressed {
    background-color: #0c0c20;
    border-color: #5858b0;
    padding-top: 14px;
    padding-bottom: 10px;
}

    )");
}