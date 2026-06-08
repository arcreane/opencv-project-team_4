#include "DeepLearningWindow.h"
#include "ui_DeepLearningWindow.h"
#include <QDebug>
#include <QPushButton>  
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
//end of all the include necessary for the deep learning window
// Important: use of the QDebug in order to understand where the program is at, and to check if the files are correctly loaded (weights, cfg, names)

DeepLearningWindow::DeepLearningWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::DeepLearningWindow) {
	ui->setupUi(this);
	//to make the window resizable and to apply the styles to the different widgets
	applyStyles();
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    centralWidget()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    backBtn = new QPushButton("Back", this); 
    backBtn->setObjectName("BackButton");
    backBtn->setGeometry(0, 0, 100, 40);
    backBtn->raise();
    connect(backBtn, &QPushButton::clicked, this, &DeepLearningWindow::backToStartInterface);
    resize(1000, 700);

    //setting of the interface for deep learning
	ui->labelOutput->setVisible(false);
    ui->labelDropDeep->setGrayscaleOnly(false);
    ui->labelDropDeep->setFixPosition(true);
    ui->savedButton->setVisible(false);
    qDebug() << "Working directory:" << QDir::currentPath();

    QString weightsPath = QDir::currentPath() + "/yolov3.weights";
    QString cfgPath = QDir::currentPath() + "/yolov3.cfg";
    QString namesPath = QDir::currentPath() + "/coco.names";

    //checking for the files if they are present or not
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

	//if file OK, we load the class names and we connect the signals for the drop and the button
    qDebug() << "YOLOv3 loaded OK";
    loadClassNames(namesPath.toStdString());

    qDebug() << ui->labelDropDeep->metaObject()->className();
	//use of the class ImageDropLabel in order to get the image dropped and to enable the button for the detection and make it readable by openCV
	connect(ui->labelDropDeep, &ImageDropLabel::imageDropped,this, [this](const QImage& img) {originalImage = img;ui->detectObjectDeep->setEnabled(true);});
	connect(ui->detectObjectDeep, &QPushButton::clicked,this, &DeepLearningWindow::deepLearningapply);
    connect(ui->savedButton, &QPushButton::clicked, this, &DeepLearningWindow::saveImage);
}

//Go back to the interface of the start, and close the current window
void DeepLearningWindow::backToStartInterface() {
    this->close();
    StartInterface* start = new StartInterface(this);
    start->show();
}

void DeepLearningWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    repositionWidgets();
}

//Function to set the different geomtry of the widgets in order to make the interface responsive and to adapt to the size of the window
void DeepLearningWindow::repositionWidgets() {
	//Base values for the geometry
    int Width = this->width();   
    int Height = this->height();

    int marginX = Width * 0.04;
    int marginY = Height * 0.05;
    int titleWidth = ui->titleDeepLearning->width();

    // Title
    int titleH = Height * 0.07;
    int usableW = Width - 2 * marginX;
    int usableH = Height - titleH - marginY * 2;
	//Columns for drag and drop and output
    int gap = Width * 0.02;
    int colW = (usableW - gap) / 2;
    int btnH = usableH * 0.08;
    int dropH = usableH * 0.80;
    int rowY = titleH + marginY;

    int c1 = marginX;
    int c2 = c1 + colW + gap;
	// Set geometry for labels and buttons. Attention: setGeometry (x, y, width, height)
    ui->titleDeepLearning->setGeometry(c2-2*c1, marginY * 0.3, titleWidth, titleH);
    ui->titleDeepLearning->setAlignment(Qt::AlignCenter);
    ui->labelDropDeep->setGeometry(c1, rowY, colW, dropH);
    ui->labelOutput->setGeometry(c2, rowY, colW, dropH);

    int btnW = colW;  
    int btnY = rowY + dropH + gap;

	//Elements shown after the detection
    ui->detectObjectDeep->setGeometry(c1, btnY, btnW, btnH);  
    ui->savedButton->setGeometry(c2, btnY, btnW, btnH);
    backBtn->setGeometry(0, 0, 200, 40);
    backBtn->raise();
}

//Function to save the image with the detection, using a QFileDialog to choose the location and the name of the file
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

//Function to load the class names from the coco.names file, and store them in a vector of strings
void DeepLearningWindow::loadClassNames(const std::string& path) {
    std::ifstream file(path);
    std::string line;
	// Check if the file is open, put inside the vector each line of the file, i go through all the lines of the file and I push them in the vector classNames, then I print the number of classes loaded
    while (std::getline(file, line))
        classNames.push_back(line);
    qDebug() << "Loaded" << classNames.size() << "classes";
}
  
void DeepLearningWindow::deepLearningapply() {
	//setting up the interface for the detection, and checking if the image and the model are loaded
    ui->labelOutput->setVisible(false);
    ui->labelOutput->clear();
    ui->labelOutput->setText("");
    ui->labelOutput->setStyleSheet("");
    if (originalImage.isNull() || net.empty()) return;
	ui->detectObjectDeep->setEnabled(false);
	ui->detectObjectDeep->setText("Charging....");
	ui->savedButton->setVisible(true);

	// Process events to update the UI before heavy computation
    QApplication::processEvents();

    // QImage → cv::Mat BGR
    QImage rgb = originalImage.convertToFormat(QImage::Format_RGB888);
    cv::Mat imgMat(rgb.height(), rgb.width(), CV_8UC3,const_cast<uchar*>(rgb.bits()), rgb.bytesPerLine());
    cv::Mat bgrMat;
    cv::cvtColor(imgMat, bgrMat, cv::COLOR_RGB2BGR);

	// Blob for YOLOv3 (416x416), using the dnn module of OpenCV, and set it as input for the network
    cv::Mat blob = cv::dnn::blobFromImage(bgrMat, 1.0 / 255.0,cv::Size(416, 416), cv::Scalar(), true, false);
    net.setInput(blob);

    // Retrieve the names of the output layers
    std::vector<std::string> outLayerNames = net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outputs;

	// Forward pass through the network, and set the button to "Detect new object" and enable it, and make the save button visible, then reposition the widgets
    net.forward(outputs, outLayerNames);
    ui->detectObjectDeep->setEnabled(true);
    ui->detectObjectDeep->setText("Detect new object");
    ui->savedButton->setVisible(true);
    repositionWidgets();

	// Parse les détections, definition of the boxes to visualize the results, 
    // the scores and the classIds, and setting of the thresholds for the confidence and for the non-maximum suppression
    std::vector<cv::Rect> boxes;
    std::vector<float>    scores;
    std::vector<int>      classIds;

	//float confThreshold = 0.5f; // confidence threshold
    float confThreshold = 0.5f;
    float nmsThreshold = 0.45f;

    for (auto& output : outputs) {
        for (int i = 0; i < output.rows; i++) {
            float* data = output.ptr<float>(i);
            cv::Mat classMat(1, (int)classNames.size(), CV_32F, data + 5);
            cv::Point classId;
            double maxScore;
			// Get the class with the highest score for this detection
            cv::minMaxLoc(classMat, nullptr, &maxScore, nullptr, &classId);

			//if the detection and its score is higher then the confident, print the boxes and the scores, and the classIds,
            // and push them in the vector of boxes, scores and classIds
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

	// Apply non-maximum suppression to filter overlapping boxes
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, scores, confThreshold, nmsThreshold, indices);
    //print the error if no detection
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

	//Otherswise, we draw the boxes and the labels on the image, and we display the result in the output label
    cv::Mat result = bgrMat.clone();
    for (int idx : indices) {
        cv::rectangle(result, boxes[idx], cv::Scalar(0, 255, 0), 2);
        //print label + score
        std::string label = classNames[classIds[idx]]+ " " + std::to_string((int)(scores[idx] * 100)) + "%";
        cv::putText(result, label,
            cv::Point(boxes[idx].x, boxes[idx].y - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.6,
            cv::Scalar(0, 255, 0), 2);
    }

    // Affiche
    cv::Mat rgbResult;
	// Convert BGR to RGB for QImage
    cv::cvtColor(result, rgbResult, cv::COLOR_BGR2RGB);
	// Create QImage from the result, from CV to QImage in order to send it to the label Output
    QImage qResult(rgbResult.data, rgbResult.cols, rgbResult.rows,rgbResult.step, QImage::Format_RGB888);


    ui->labelOutput->setGeometry(
        dropGeom.x() + dropGeom.width() + (int)(this->width() * 0.02),
        dropGeom.y(),
        dropGeom.width() + (int)(this->width() * 0.02),
        dropGeom.height()
    );
    ui->labelOutput->setVisible(true);

	// Scale the pixmap to fit the label while keeping aspect ratio
    QPixmap pix = QPixmap::fromImage(qResult.copy()).scaled(ui->labelOutput->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    ui->labelOutput->setPixmap(pix);

	// Reposition widgets after processing to ensure the output label is correctly placed, especially if the window was resized during processing
    QTimer::singleShot(0, this, [this, dropGeom]() {
        ui->labelDropDeep->setGeometry(dropGeom);  
        ui->labelOutput->setGeometry(
            dropGeom.x() + dropGeom.width() + (int)(this->width() * 0.02),
            dropGeom.y(),
            dropGeom.width(),
            dropGeom.height()
        );
        });
}

//CSS
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