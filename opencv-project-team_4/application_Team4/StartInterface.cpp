#include "StartInterface.h"
#include "ui_StartInterface.h"
#include "MorphologyWindow.h"
#include "DragDropCannyEdge.h"
#include "DeepLearningWindow.h"
#include "HistogramEqualization.h"
#include "ThresholdingWindow.h"
#include "InteractiveToolsWindow.h"
#include "CreativeEffectsWindow.h"



#include <QDebug>
StartInterface::StartInterface(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::StartInterface) {
	ui->setupUi(this);

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
	ThresholdingWindow* w = new ThresholdingWindow(this);
	w->show();
	hide();
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
	InteractiveToolsWindow* inter = new InteractiveToolsWindow(this);
	inter->show();
	hide();
}
void StartInterface::goToCreativeInterface() {
	CreativeEffectsWindow* creative = new CreativeEffectsWindow(this);
	creative->show();
	hide();
}
