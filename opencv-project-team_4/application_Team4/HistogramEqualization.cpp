#include "HistogramEqualization.h"
#include "ui_HistogramEqualization.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QString>
#include "StartInterface.h"

HistogramEqualizationWindow::HistogramEqualizationWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HistogramEqualizationWindow)
	, m_startInterface(nullptr)
{
    ui->setupUi(this);
	applyStyles();
    setWindowTitle("Histogram Equalization");

    ui->clipLimitSlider->setRange(10, 80);      // displayed as 1.0 to 8.0
    ui->clipLimitSlider->setValue(20);          // default 2.0

    ui->tileGridSizeSlider->setRange(2, 32);
    ui->tileGridSizeSlider->setValue(8);        // default 8x8 tiles

    onClipLimitChanged(ui->clipLimitSlider->value());
    onTileGridSizeChanged(ui->tileGridSizeSlider->value());

    connect(ui->loadButton, &QPushButton::clicked,
            this, &HistogramEqualizationWindow::onLoadImage);
    connect(ui->globalEqualizationButton, &QPushButton::clicked,
            this, &HistogramEqualizationWindow::onApplyGlobalEqualization);
    connect(ui->claheButton, &QPushButton::clicked,
            this, &HistogramEqualizationWindow::onApplyCLAHE);
    connect(ui->resetButton, &QPushButton::clicked,
            this, &HistogramEqualizationWindow::onResetImage);
    connect(ui->saveButton, &QPushButton::clicked,
            this, &HistogramEqualizationWindow::onSaveResult);
    connect(ui->backButton, &QPushButton::clicked,
            this, &HistogramEqualizationWindow::onBack);

    connect(ui->clipLimitSlider, &QSlider::valueChanged,
            this, &HistogramEqualizationWindow::onClipLimitChanged);
    connect(ui->tileGridSizeSlider, &QSlider::valueChanged,
            this, &HistogramEqualizationWindow::onTileGridSizeChanged);

    ui->statusLabel->setText("Load an image to begin.");
}

HistogramEqualizationWindow::~HistogramEqualizationWindow()
{
    delete ui;
}

void HistogramEqualizationWindow::onLoadImage()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open image",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;All files (*.*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    cv::Mat loaded = cv::imread(fileName.toStdString(), cv::IMREAD_UNCHANGED);

    if (loaded.empty()) {
        QMessageBox::warning(this, "Load error", "Could not load this image.");
        return;
    }

    // Normalize supported formats to 8-bit grayscale, BGR, or BGRA.
    if (loaded.depth() != CV_8U) {
        double minValue = 0.0;
        double maxValue = 0.0;
        cv::minMaxLoc(loaded, &minValue, &maxValue);
        if (maxValue > minValue) {
            loaded.convertTo(loaded, CV_8U, 255.0 / (maxValue - minValue), -minValue * 255.0 / (maxValue - minValue));
        } else {
            loaded.convertTo(loaded, CV_8U);
        }
    }

    m_originalImage = loaded.clone();
    m_resultImage = loaded.clone();

    updatePreview();
    ui->statusLabel->setText("Image loaded. Choose global equalization or CLAHE.");
}

void HistogramEqualizationWindow::onApplyGlobalEqualization()
{
    if (!imageLoaded()) {
        QMessageBox::information(this, "No image", "Please load an image first.");
        return;
    }

    try {
        m_resultImage = applyGlobalEqualization(m_originalImage);
        updatePreview();
        ui->statusLabel->setText("Applied global histogram equalization.");
    } catch (const cv::Exception &error) {
        QMessageBox::critical(this, "OpenCV error", error.what());
    }
}

void HistogramEqualizationWindow::onApplyCLAHE()
{
    if (!imageLoaded()) {
        QMessageBox::information(this, "No image", "Please load an image first.");
        return;
    }

    try {
        m_resultImage = applyCLAHEEqualization(
            m_originalImage,
            currentClipLimit(),
            currentTileGridSize()
        );

        updatePreview();
        ui->statusLabel->setText(
            QString("Applied CLAHE: clip limit %1, tile grid %2x%2.")
                .arg(currentClipLimit(), 0, 'f', 1)
                .arg(currentTileGridSize())
        );
    } catch (const cv::Exception &error) {
        QMessageBox::critical(this, "OpenCV error", error.what());
    }
}

void HistogramEqualizationWindow::onResetImage()
{
    if (!imageLoaded()) {
        return;
    }

    m_resultImage = m_originalImage.clone();
    updatePreview();
    ui->statusLabel->setText("Result reset to original image.");
}

void HistogramEqualizationWindow::onSaveResult()
{
    if (m_resultImage.empty()) {
        QMessageBox::information(this, "No result", "There is no result image to save.");
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save result",
        "histogram_equalization_result.png",
        "PNG image (*.png);;JPEG image (*.jpg *.jpeg);;BMP image (*.bmp);;All files (*.*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    if (!cv::imwrite(fileName.toStdString(), m_resultImage)) {
        QMessageBox::warning(this, "Save error", "Could not save the result image.");
        return;
    }

    ui->statusLabel->setText("Result image saved successfully.");
}

void HistogramEqualizationWindow::onBack()
{
    this->close();
    StartInterface* start = new StartInterface(this);
    start->show();
}

void HistogramEqualizationWindow::onClipLimitChanged(int value)
{
    ui->clipLimitValueLabel->setText(QString::number(value / 10.0, 'f', 1));
}

void HistogramEqualizationWindow::onTileGridSizeChanged(int value)
{
    ui->tileGridSizeValueLabel->setText(QString("%1 x %1").arg(value));
}

void HistogramEqualizationWindow::updatePreview()
{
    displayMatOnLabel(m_originalImage, ui->originalImageLabel);
    displayMatOnLabel(m_resultImage, ui->resultImageLabel);
}

void HistogramEqualizationWindow::displayMatOnLabel(const cv::Mat &image, QLabel *label)
{
    if (image.empty() || label == nullptr) {
        return;
    }

    const QImage qImage = matToQImage(image);
    label->setPixmap(
        QPixmap::fromImage(qImage).scaled(
            label->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}

QImage HistogramEqualizationWindow::matToQImage(const cv::Mat &image) const
{
    if (image.empty()) {
        return QImage();
    }

    if (image.type() == CV_8UC1) {
        QImage qImage(image.data, image.cols, image.rows, static_cast<int>(image.step), QImage::Format_Grayscale8);
        return qImage.copy();
    }

    if (image.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
        QImage qImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        return qImage.copy();
    }

    if (image.type() == CV_8UC4) {
        cv::Mat rgba;
        cv::cvtColor(image, rgba, cv::COLOR_BGRA2RGBA);
        QImage qImage(rgba.data, rgba.cols, rgba.rows, static_cast<int>(rgba.step), QImage::Format_RGBA8888);
        return qImage.copy();
    }

    cv::Mat converted;
    image.convertTo(converted, CV_8U);
    return matToQImage(converted);
}

cv::Mat HistogramEqualizationWindow::applyGlobalEqualization(const cv::Mat &image) const
{
    cv::Mat result;

    if (image.channels() == 1) {
        cv::equalizeHist(image, result);
        return result;
    }

    cv::Mat bgrImage;
    if (image.channels() == 4) {
        cv::cvtColor(image, bgrImage, cv::COLOR_BGRA2BGR);
    } else {
        bgrImage = image;
    }

    // Equalize only luminance to avoid destroying the original colors.
    cv::Mat yCrCb;
    cv::cvtColor(bgrImage, yCrCb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(yCrCb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, yCrCb);

    cv::cvtColor(yCrCb, result, cv::COLOR_YCrCb2BGR);
    return result;
}

cv::Mat HistogramEqualizationWindow::applyCLAHEEqualization(const cv::Mat &image, double clipLimit, int tileGridSize) const
{
    cv::Mat result;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(
        clipLimit,
        cv::Size(tileGridSize, tileGridSize)
    );

    if (image.channels() == 1) {
        clahe->apply(image, result);
        return result;
    }

    cv::Mat bgrImage;
    if (image.channels() == 4) {
        cv::cvtColor(image, bgrImage, cv::COLOR_BGRA2BGR);
    } else {
        bgrImage = image;
    }

    // Apply CLAHE locally on the luminance channel only.
    cv::Mat yCrCb;
    cv::cvtColor(bgrImage, yCrCb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(yCrCb, channels);
    clahe->apply(channels[0], channels[0]);
    cv::merge(channels, yCrCb);

    cv::cvtColor(yCrCb, result, cv::COLOR_YCrCb2BGR);
    return result;
}

bool HistogramEqualizationWindow::imageLoaded() const
{
    return !m_originalImage.empty();
}

double HistogramEqualizationWindow::currentClipLimit() const
{
    return ui->clipLimitSlider->value() / 10.0;
}

int HistogramEqualizationWindow::currentTileGridSize() const
{
    return std::max(2, ui->tileGridSizeSlider->value());
}
void HistogramEqualizationWindow::applyStyles()
{
    setStyleSheet(R"(
QMainWindow { background-color: #0d0d1c; }
QWidget#centralwidget { background-color: #0d0d1c; }

QGroupBox {
    color: #50508a;
    font-family: 'Segoe UI';
    font-size: 10px;
    font-weight: bold;
    letter-spacing: 1px;
    border: 1px solid #1a1a38;
    border-radius: 8px;
    margin-top: 10px;
    padding-top: 8px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
    color: #50508a;
}

QLabel {
    color: #c0c0e0;
    font-family: 'Segoe UI';
    font-size: 12px;
    background: transparent;
}
QLabel#originalImageLabel, QLabel#resultImageLabel {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 6px;
    color: #30304c;
    font-size: 14px;
}
QLabel#titleLabel {
    color: #d0d0ff;
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 3px;
    background: transparent;
}
QLabel#statusLabel {
    color: #8888aa;
    background: #0a0a18;
    border: 1px solid #1c1c38;
    border-radius: 6px;
    padding: 0 10px;
    font-size: 12px;
}

QSlider::groove:horizontal {
    background: #1a1a38;
    height: 4px;
    border-radius: 2px;
}
QSlider::handle:horizontal {
    background: #6060c0;
    border: 1px solid #4040a0;
    width: 14px;
    height: 14px;
    margin: -5px 0;
    border-radius: 7px;
}
QSlider::handle:horizontal:hover { background: #8080e0; }
QSlider::sub-page:horizontal {
    background: #3838a0;
    border-radius: 2px;
}

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

QPushButton#loadButton {
    background-color: #0a2518;
    border-color: #185a35;
    color: #48c880;
}
QPushButton#loadButton:hover {
    background-color: #103020;
    border-color: #287a50;
    color: #68e8a0;
}

QPushButton#globalEqualizationButton, QPushButton#claheButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff;
    border: none;
    border-radius: 10px;
    font-size: 13px;
    font-weight: bold;
}
QPushButton#globalEqualizationButton:hover, QPushButton#claheButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}

QPushButton#resetButton {
    background-color: #250a0a;
    border-color: #5a1818;
    color: #c84848;
}
QPushButton#resetButton:hover {
    background-color: #301010;
    border-color: #7a2828;
    color: #e86868;
}

QPushButton#saveButton {
    background-color: #0a1828;
    border-color: #183868;
    color: #4898d0;
}
QPushButton#saveButton:hover {
    background-color: #101e32;
    border-color: #285898;
    color: #68b8f0;
}

QPushButton#backButton {
    background-color: #181424;
    border-color: #362850;
    color: #8860b8;
}
QPushButton#backButton:hover {
    background-color: #20182e;
    border-color: #503878;
    color: #a888d8;
}

QScrollBar:vertical {
    background: #07070f; width: 6px; margin: 0; border-radius: 3px;
}
QScrollBar::handle:vertical {
    background: #252558; border-radius: 3px; min-height: 20px;
}
QScrollBar::handle:vertical:hover { background: #3838a0; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QScrollBar:horizontal {
    background: #07070f; height: 6px; border-radius: 3px;
}
QScrollBar::handle:horizontal {
    background: #252558; border-radius: 3px;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
    )");
}