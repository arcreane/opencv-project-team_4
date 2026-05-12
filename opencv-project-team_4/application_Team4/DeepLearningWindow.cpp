#include "DeepLearningWindow.h"
#include "ui_DeepLearningWindow.h"


#include <QDebug>
DragAndDropWindow::DragAndDropWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::DeepLearningWindow) {
	ui->setupUi(this);
	ui->connect(ui->detectObjectDeep, &QPushButton::clicked, this, [this]() {
		deepLearningapply();
	});
	
	
}

DragAndDropWindow::~DragAndDropWindow() {
	delete ui;
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
 
void deepLearningapply() {
	// Load the pre-trained model
	cv::dnn::Net net = cv::dnn::readNetFromONNX("model.onnx");
	// Convert the input image to a blob
	cv::Mat inputBlob = cv::dnn::blobFromImage(QImageToMat(A METTRE ORIGINAL IMAGE), 1.0, cv::Size(224, 224), cv::Scalar(104, 117, 123));
	// Set the input blob for the network
	net.setInput(inputBlob);
	// Perform forward pass to get the output
	cv::Mat output = net.forward();
	// Process the output and display results (this is just a placeholder)
	qDebug() << "Output shape: " << output.size();
}