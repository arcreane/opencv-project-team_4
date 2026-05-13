#include "DeepLearningWindow.h"
#include "ui_DeepLearningWindow.h"
#include <QDebug>
#include <QDir>

#include "imagedropLabel.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>   
#include <string> 


DeepLearningWindow::DeepLearningWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::DeepLearningWindow) {
	ui->setupUi(this);
	ui->labelOutput->setVisible(false);
    qDebug() << "Working directory:" << QDir::currentPath();

    QString weightsPath = QDir::currentPath() + "/yolov3.weights";
    QString cfgPath = QDir::currentPath() + "/yolov3.cfg";
    QString namesPath = QDir::currentPath() + "/coco.names";

    qDebug() << "Looking for weights:" << weightsPath;
    qDebug() << "weights exists:" << QFile::exists(weightsPath);
    qDebug() << "cfg exists:" << QFile::exists(cfgPath);
    qDebug() << "names exists:" << QFile::exists(namesPath);

    if (!QFile::exists(weightsPath) || !QFile::exists(cfgPath)) {
        qDebug() << "ERROR: model files missing — copy them to the folder above";
        return;
    }

    net = cv::dnn::readNet(weightsPath.toStdString(), cfgPath.toStdString());
    if (net.empty()) {
        qDebug() << "ERROR: model failed to load";
        return;
    }
    qDebug() << "YOLOv3 loaded OK";
    loadClassNames(namesPath.toStdString());
    ui->labelDropDeep->setGrayscaleOnly(false);

	connect(ui->labelDropDeep, &ImageDropLabel::imageDropped,
		this, [this](const QImage& img) {
			originalImage = img;
			ui->detectObjectDeep->setEnabled(true);
		});

	connect(ui->detectObjectDeep, &QPushButton::clicked,
		this, &DeepLearningWindow::deepLearningapply);
	
}

DeepLearningWindow::~DeepLearningWindow() {
	delete ui;
}
void DeepLearningWindow::loadClassNames(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line))
        classNames.push_back(line);
    qDebug() << "Loaded" << classNames.size() << "classes";
}
  
void DeepLearningWindow::deepLearningapply() {
    if (originalImage.isNull() || net.empty()) return;

    // QImage → cv::Mat BGR
    QImage rgb = originalImage.convertToFormat(QImage::Format_RGB888);
    cv::Mat imgMat(rgb.height(), rgb.width(), CV_8UC3,
        const_cast<uchar*>(rgb.bits()), rgb.bytesPerLine());
    cv::Mat bgrMat;
    cv::cvtColor(imgMat, bgrMat, cv::COLOR_RGB2BGR);

    // Blob pour YOLOv3 (416x416)
    cv::Mat blob = cv::dnn::blobFromImage(bgrMat, 1.0 / 255.0,
        cv::Size(416, 416), cv::Scalar(), true, false);
    net.setInput(blob);

    // Récupère les noms des couches de sortie
    std::vector<std::string> outLayerNames = net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outputs;
    net.forward(outputs, outLayerNames);

    // Parse les détections
    std::vector<cv::Rect> boxes;
    std::vector<float>    scores;
    std::vector<int>      classIds;
    float confThreshold = 0.5f;
    float nmsThreshold = 0.45f;

    for (auto& output : outputs) {
        for (int i = 0; i < output.rows; i++) {
            float* data = output.ptr<float>(i);
            cv::Mat classMat(1, (int)classNames.size(), CV_32F, data + 5);
            cv::Point classId;
            double maxScore;
            cv::minMaxLoc(classMat, nullptr, &maxScore, nullptr, &classId);

            if (data[4] * maxScore >= confThreshold) {
                int cx = (int)(data[0] * bgrMat.cols);
                int cy = (int)(data[1] * bgrMat.rows);
                int w = (int)(data[2] * bgrMat.cols);
                int h = (int)(data[3] * bgrMat.rows);
                boxes.push_back(cv::Rect(cx - w / 2, cy - h / 2, w, h));
                scores.push_back((float)(data[4] * maxScore));
                classIds.push_back(classId.x);
            }
        }
    }

    // NMS
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, scores, confThreshold, nmsThreshold, indices);

    // Dessine les résultats
    cv::Mat result = bgrMat.clone();
    for (int idx : indices) {
        cv::rectangle(result, boxes[idx], cv::Scalar(0, 255, 0), 2);
        std::string label = classNames[classIds[idx]]
            + " " + std::to_string((int)(scores[idx] * 100)) + "%";
        cv::putText(result, label,
            cv::Point(boxes[idx].x, boxes[idx].y - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.6,
            cv::Scalar(0, 255, 0), 2);
    }

    // Affiche
    cv::Mat rgbResult;
    cv::cvtColor(result, rgbResult, cv::COLOR_BGR2RGB);
    QImage qResult(rgbResult.data, rgbResult.cols, rgbResult.rows,
        rgbResult.step, QImage::Format_RGB888);

    ui->labelOutput->setVisible(true);
    ui->labelOutput->setPixmap(
        QPixmap::fromImage(qResult.copy()).scaled(
            ui->labelOutput->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}