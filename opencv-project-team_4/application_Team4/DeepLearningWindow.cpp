#include "DeepLearningWindow.h"
#include "ui_DeepLearningWindow.h"
#include <QDebug>
#include <QDir>
#include <QTimer>
#include "imagedropLabel.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>   
#include <string> 
#include <QFileDialog>


DeepLearningWindow::DeepLearningWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::DeepLearningWindow) {
	ui->setupUi(this);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    centralWidget()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    resize(1000, 700);
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
    ui->labelDropDeep->setFixPosition(true);
	ui->savedButton->setVisible(false);
    qDebug() << ui->labelDropDeep->metaObject()->className();
	connect(ui->labelDropDeep, &ImageDropLabel::imageDropped,
		this, [this](const QImage& img) {
			originalImage = img;
			ui->detectObjectDeep->setEnabled(true);
		});

	connect(ui->detectObjectDeep, &QPushButton::clicked,this, &DeepLearningWindow::deepLearningapply);
    connect(ui->savedButton, &QPushButton::clicked, this, &DeepLearningWindow::saveImage);

	
}


void DeepLearningWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    repositionWidgets();
}

void DeepLearningWindow::repositionWidgets() {
    int Width = this->width();   
    int Height = this->height();

    int marginX = Width * 0.04;
    int marginY = Height * 0.05;
    int titleWidth = ui->titleDeepLearning->width();
    // Titre — pleine largeur
    int titleH = Height * 0.07;
    int usableW = Width - 2 * marginX;
    int usableH = Height - titleH - marginY * 2;
    // 2 colonnes égales : drop | output
    int gap = Width * 0.02;
    int colW = (usableW - gap) / 2;
    int btnH = usableH * 0.08;
    int dropH = usableH * 0.80;
    int rowY = titleH + marginY;

    int c1 = marginX;
    int c2 = c1 + colW + gap;
    
    ui->titleDeepLearning->setGeometry(c2-2*c1, marginY * 0.3, titleWidth, titleH);
    ui->titleDeepLearning->setAlignment(Qt::AlignCenter);

    ui->labelDropDeep->setGeometry(c1, rowY, colW, dropH);
    //ui->labelDropDeep->setFixedSize(colW, dropH);


    ui->labelOutput->setGeometry(c2, rowY, colW, dropH);
    //ui->labelOutput->setFixedSize(colW, dropH);
    int totalBtnW = c2 + colW - c1;
    int btnW = (totalBtnW - 2 * gap) / 3;
    int btnY = rowY + dropH + gap;

    ui->detectObjectDeep->setGeometry(c1, btnY, btnW, btnH);
    ui->savedButton->setGeometry(c2 + btnW, btnY, btnW, btnH);
}
void DeepLearningWindow::saveImage() {
    QPixmap pix = ui->labelOutput->pixmap(Qt::ReturnByValue);
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
	ui->detectObjectDeep->setText("Detect new object");
	ui->savedButton->setVisible(true);
	repositionWidgets();
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
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, scores, confThreshold, nmsThreshold, indices);
    if (indices.empty()) {

        ui->labelOutput->setVisible(true);
        ui->labelOutput->setStyleSheet("border: 2px dashed gray;");
        ui->labelOutput->setText(
            "No detection found.\nTry with another picture."
        );

        ui->labelOutput->setAlignment(Qt::AlignCenter);

        ui->labelOutput->setStyleSheet(
            "border: 2px solid red;"
            "color: red;"
            "font-size: 18px;"
            "font-weight: bold;"
            "background-color: white;"
        );

        return;
    }
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

    QRect dropGeom = ui->labelDropDeep->geometry();  // sauvegarder AVANT

    ui->labelOutput->setGeometry(
        dropGeom.x() + dropGeom.width() + (int)(this->width() * 0.02),
        dropGeom.y(),
        dropGeom.width(),
        dropGeom.height()
    );
    ui->labelOutput->setVisible(true);

    QPixmap pix = QPixmap::fromImage(qResult.copy()).scaled(
        ui->labelOutput->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    ui->labelOutput->setPixmap(pix);

    // Restaurer après setPixmap
    QTimer::singleShot(0, this, [this, dropGeom]() {
        ui->labelDropDeep->setGeometry(dropGeom);  // remet labelDropDeep à sa place
        ui->labelOutput->setGeometry(
            dropGeom.x() + dropGeom.width() + (int)(this->width() * 0.02),
            dropGeom.y(),
            dropGeom.width(),
            dropGeom.height()
        );
        });
}