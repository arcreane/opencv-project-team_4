#include "StartInterface.h"
#include "ui_StartInterface.h"


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
	// Implement the logic to navigate to the Morphology Interface
	
}
void StartInterface::goToThresholdingInterface() {
	// Implement the logic to navigate to the Thresholding Interface
	
}
void StartInterface::goToCannyEdgeInterface() {
	// Implement the logic to navigate to the Canny Edge Detection Interface
	
}
void StartInterface::goToGeometricInterface() {
	// Implement the logic to navigate to the Geometric Transformations Interface
	
}
void StartInterface::goToPanoramaInterface() {
	// Implement the logic to navigate to the Panorama Stitching Interface
	
}
void StartInterface::goToHistogramInterface() {
	// Implement the logic to navigate to the Histogram Interface
	
}
void StartInterface::goToVideoInterface() {
	// Implement the logic to navigate to the Video Processing Interface
	
}
void StartInterface::goToDeepLearningInterface() {
	// Implement the logic to navigate to the Deep Learning Interface
	
}
void StartInterface::goToInteractiveInterface() {
	// Implement the logic to navigate to the Interactive Interface
	
}
void StartInterface::goToCreativeInterface() {
	// Implement the logic to navigate to the Creative Interface
	
}
