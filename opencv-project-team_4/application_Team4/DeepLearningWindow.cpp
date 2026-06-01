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
#include <QMessageBox>
#include <QApplication>
#include "StartInterface.h"

DeepLearningWindow::DeepLearningWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::DeepLearningWindow) {
	ui->setupUi(this);
	applyStyles();
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    centralWidget()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    resize(1000, 700);
	ui->labelOutput->setVisible(false);
    ui->labelDropDeep->setGrayscaleOnly(false);
    ui->labelDropDeep->setFixPosition(true);
    ui->savedButton->setVisible(false);
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
		ui->labelDropDeep->setStyleSheet("border: 2px dashed red; color: red;");
        ui->labelDropDeep->setText(
            "Model files not found. Alert the team."
		);
        return;
    }

    net = cv::dnn::readNet(weightsPath.toStdString(), cfgPath.toStdString());
    if (net.empty()) {
        qDebug() << "ERROR: model failed to load";
        ui->labelDropDeep->setStyleSheet("border: 2px dashed red; color: red;");
        ui->labelDropDeep->setText(
            "Model files not found. Alert the team."
        );
        return;
    }
    qDebug() << "YOLOv3 loaded OK";
    loadClassNames(namesPath.toStdString());

    qDebug() << ui->labelDropDeep->metaObject()->className();
	connect(ui->labelDropDeep, &ImageDropLabel::imageDropped,
		this, [this](const QImage& img) {
			originalImage = img;
			ui->detectObjectDeep->setEnabled(true);
		});

	connect(ui->detectObjectDeep, &QPushButton::clicked,this, &DeepLearningWindow::deepLearningapply);
    connect(ui->savedButton, &QPushButton::clicked, this, &DeepLearningWindow::saveImage);
	connect(ui->BackButton, &QPushButton::clicked, this, &DeepLearningWindow::backToStartInterface);
}
void DeepLearningWindow::backToStartInterface() {
    this->close();
    StartInterface* start = new StartInterface(this);
    start->show();
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
    int btnW = colW;  // même largeur que les labels
    int btnY = rowY + dropH + gap;

    ui->detectObjectDeep->setGeometry(c1, btnY, btnW, btnH);  // aligné sous labelDropDeep
    ui->savedButton->setGeometry(c2, btnY, btnW, btnH);
    ui->BackButton->setParent(this);
    ui->BackButton->setGeometry(0, 0, 200, 40);
    ui->BackButton->raise();
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
    ui->labelOutput->setVisible(false);
    ui->labelOutput->clear();
    ui->labelOutput->setText("");
    ui->labelOutput->setStyleSheet("");
    if (originalImage.isNull() || net.empty()) return;
	ui->detectObjectDeep->setEnabled(false);
	ui->detectObjectDeep->setText("Charging....");
	ui->savedButton->setVisible(true);
    QApplication::processEvents();
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
    ui->detectObjectDeep->setEnabled(true);
    ui->detectObjectDeep->setText("Detect new object");
    ui->savedButton->setVisible(true);
    repositionWidgets();
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
    QRect dropGeom = ui->labelDropDeep->geometry();

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, scores, confThreshold, nmsThreshold, indices);
    if (indices.empty()) {

        ui->labelOutput->setVisible(true);
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
        ui->labelOutput->setGeometry(
            dropGeom.x() + dropGeom.width() + (int)(this->width() * 0.02),
            dropGeom.y(),
            dropGeom.width() + (int)(this->width() * 0.02),
            dropGeom.height()
		); repositionWidgets();
        

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


    ui->labelOutput->setGeometry(
        dropGeom.x() + dropGeom.width() + (int)(this->width() * 0.02),
        dropGeom.y(),
        dropGeom.width() + (int)(this->width() * 0.02),
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
void DeepLearningWindow::applyStyles() {
    setStyleSheet(R"(

/* ── Window & surfaces ───────────────────────────────── */
QMainWindow {
    background-color: #0d0d1c;
}
QWidget#centralwidget {
    background-color: #0d0d1c;
}

/* ── Title label ─────────────────────────────────────── */
QLabel#titleDeepLearning {
    color: #d0d0ff;
    font-family: 'Segoe UI';
    font-size: 20px;
    font-weight: bold;
    letter-spacing: 3px;
    background: transparent;
}

/* ── Drop label ──────────────────────────────────────── */
QLabel#labelDropDeep {
    background-color: #07070f;
    border: 2px dashed #252558;
    border-radius: 10px;
    color: #40406a;
    font-family: 'Segoe UI';
    font-size: 16px;
}
QLabel#labelDropDeep:hover {
    border-color: #4848a8;
    background-color: #0a0a18;
}

/* ── Output label ────────────────────────────────────── */
QLabel#labelOutput {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 10px;
    color: #30304c;
    font-family: 'Segoe UI';
    font-size: 14px;
}

/* ── Buttons – base ──────────────────────────────────── */
QPushButton {
    background-color: #16162e;
    color: #b0b0d0;
    border: 1px solid #26264c;
    border-radius: 8px;
    padding: 6px 14px;
    font-family: 'Segoe UI';
    font-size: 12px;
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
    padding-top: 8px;
    padding-bottom: 4px;
}
QPushButton:disabled {
    background-color: #0c0c18;
    border-color: #141428;
    color: #303050;
}

/* ── Detect button – gradient accent ─────────────────── */
QPushButton#detectObjectDeep {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff;
    border: none;
    border-radius: 10px;
    font-family: 'Segoe UI';
    font-size: 14px;
    font-weight: bold;
    letter-spacing: 1px;
}
QPushButton#detectObjectDeep:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#detectObjectDeep:pressed {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #2828c0, stop:1 #6818b0);
    padding-top: 8px;
    padding-bottom: 4px;
}
QPushButton#detectObjectDeep:disabled {
    background: #141428;
    color: #303050;
}

/* ── Save button – blue accent ───────────────────────── */
QPushButton#savedButton {
    background-color: #0a1828;
    border-color: #183868;
    color: #4898d0;
    border-radius: 10px;
    font-size: 14px;
    font-weight: bold;
}
QPushButton#savedButton:hover {
    background-color: #101e32;
    border-color: #285898;
    color: #68b8f0;
}
QPushButton#savedButton:pressed {
    padding-top: 8px;
    padding-bottom: 4px;
}
/* ── Back button ─────────────────────────────────────── */
QPushButton#BackButton {
    font-family: 'Segoe UI';
    font-size: 14px;
    font-weight: bold;
    color: #d0d0ff;
    background-color: #1a1a2e;
    border: 2px solid #d0d0ff;
    border-radius: 5px;
}
QPushButton#BackButton:hover {
    background-color: #20204a;
    border-color: #ffffff;
    color: #ffffff;
}
QPushButton#BackButton:pressed {
    background-color: #0c0c20;
    padding-top: 8px;
    padding-bottom: 4px;
}
/* ── Scrollbars ──────────────────────────────────────── */
QScrollBar:vertical {
    background: #07070f;
    width: 6px;
    margin: 0;
    border-radius: 3px;
}
QScrollBar::handle:vertical {
    background: #252558;
    border-radius: 3px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover { background: #3838a0; }
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }

QScrollBar:horizontal {
    background: #07070f;
    height: 6px;
    border-radius: 3px;
}
QScrollBar::handle:horizontal {
    background: #252558;
    border-radius: 3px;
}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal { width: 0; }

/* ── Menu / status bars ──────────────────────────────── */
QMenuBar {
    background-color: #08080f;
    color: #606090;
    border-bottom: 1px solid #14142a;
}
QMenuBar::item:selected {
    background-color: #1a1a38;
    color: #c0c0e0;
}
QStatusBar {
    background-color: #08080f;
    color: #404070;
    border-top: 1px solid #14142a;
    font-size: 11px;
}

)");
}