#include "StartInterface.h"
#include "ui_StartInterface.h"


#include <QDebug>
StartInterface::StartInterface(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::StartInterface) {
	ui->setupUi(this);
	
	connect(ui->ErosionFilter, &QPushButton::clicked, this, &DragAndDropWindow::createErosionInterface);
	connect(ui->DilationFilter, &QPushButton::clicked, this, &DragAndDropWindow::createDilationInterface);
	connect(ui->OpeningFilter, &QPushButton::clicked, this, &DragAndDropWindow::createOpeningInterface);
	connect(ui->ClosingFilter, &QPushButton::clicked, this, &DragAndDropWindow::createClosingInterface);
	connect(ui->applyButton, &QPushButton::clicked, this, &DragAndDropWindow::applyCurrentFilter);



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
