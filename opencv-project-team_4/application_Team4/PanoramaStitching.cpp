#include "PanoramaStitching.h"
#include "ui_PanoramaStitching.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/stitching.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

PanoramaStitching::PanoramaStitching(QWidget* startInterface, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::PanoramaStitching)
    , m_startInterface(startInterface)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    applyStyles();

    // List widget – icon mode
    ui->imageListWidget->setViewMode(QListWidget::IconMode);
    ui->imageListWidget->setIconSize(QSize(110, 82));
    ui->imageListWidget->setResizeMode(QListWidget::Adjust);
    ui->imageListWidget->setWordWrap(true);
    ui->imageListWidget->setUniformItemSizes(true);
    ui->imageListWidget->setSpacing(4);

    // Combo box entries
    ui->modeComboBox->addItem("Panorama", static_cast<int>(cv::Stitcher::PANORAMA));
    ui->modeComboBox->addItem("Scans",    static_cast<int>(cv::Stitcher::SCANS));

    // Loading-dots timer (fires every 380 ms while stitching)
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(380);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingDots = (m_loadingDots + 1) % 4;
        setStatus("Assembly in progress" + QString(m_loadingDots, '.'), StatusType::Loading);
    });

    // Signal–slot connections
    connect(ui->addImagesButton,   &QPushButton::clicked, this, &PanoramaStitching::onAddImages);
    connect(ui->removeImageButton, &QPushButton::clicked, this, &PanoramaStitching::onRemoveImage);
    connect(ui->stitchButton,      &QPushButton::clicked, this, &PanoramaStitching::onStitch);
    connect(ui->saveButton,        &QPushButton::clicked, this, &PanoramaStitching::onSave);
    connect(ui->backButton,        &QPushButton::clicked, this, &PanoramaStitching::onBack);
}

PanoramaStitching::~PanoramaStitching() {
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────────────────────────────────────

void PanoramaStitching::onAddImages() {
    QStringList paths = QFileDialog::getOpenFileNames(
        this,
        "Select images",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff *.tif)"
    );
    if (paths.isEmpty())
        return;

    m_imagePaths.append(paths);
    refreshThumbnails();
    setStatus(QString("%1 image(s) loaded").arg(m_imagePaths.size()), StatusType::Info);
}

void PanoramaStitching::onRemoveImage() {
    QList<int> rows;
    for (QListWidgetItem* item : ui->imageListWidget->selectedItems())
        rows.append(ui->imageListWidget->row(item));

    if (rows.isEmpty()) {
        setStatus("No image selected.", StatusType::Info);
        return;
    }

    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows) {
        m_imagePaths.removeAt(row);
        delete ui->imageListWidget->takeItem(row);
    }

    setStatus(QString("%1 image(s) loaded").arg(m_imagePaths.size()), StatusType::Info);
}

void PanoramaStitching::onStitch() {
    if (m_imagePaths.size() < 2) {
        QMessageBox::warning(this, "Error", "Load at least 2 images");
        return;
    }

    // UI: disable controls, start loading animation
    ui->stitchButton->setEnabled(false);
    ui->stitchButton->setText("Assembling...");
    ui->saveButton->setEnabled(false);
    m_loadingDots = 0;
    m_loadingTimer->start();
    QApplication::processEvents();

    // Load images via OpenCV
    std::vector<cv::Mat> images;
    images.reserve(static_cast<size_t>(m_imagePaths.size()));
    for (const QString& path : m_imagePaths) {
        cv::Mat img = cv::imread(path.toStdString());
        if (img.empty()) {
            m_loadingTimer->stop();
            ui->stitchButton->setEnabled(true);
            ui->stitchButton->setText("Stitch");
            QMessageBox::warning(this, "Error",
                QString("Impossible to load image :\n%1").arg(path));
            setStatus("Fail : unreadable image.", StatusType::Error);
            return;
        }
        images.push_back(img);
    }

    // Stitch
    int modeVal = ui->modeComboBox->currentData().toInt();
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(
        static_cast<cv::Stitcher::Mode>(modeVal));
    cv::Mat result;
    cv::Stitcher::Status status = stitcher->stitch(images, result);

    // Restore UI controls
    m_loadingTimer->stop();
    ui->stitchButton->setEnabled(true);
    ui->stitchButton->setText("Stitch");

    switch (status) {
    case cv::Stitcher::OK:
        m_result = result;
        showResultWithFadeIn(
            matToPixmap(m_result).scaled(
                ui->resultLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
        ui->saveButton->setEnabled(true);
        setStatus("Stitch succeeded !", StatusType::Success);
        break;

    case cv::Stitcher::ERR_NEED_MORE_IMGS:
        QMessageBox::warning(this, "Stitching Error",
            "Not enough images or not enough overlap between images.");
        setStatus("Fail : not enough overlap.", StatusType::Error);
        break;

    case cv::Stitcher::ERR_HOMOGRAPHY_EST_FAIL:
        QMessageBox::warning(this, "Stitching Error",
            "Homography estimation failed.");
        setStatus("Fail : homography estimation impossible.", StatusType::Error);
        break;

    case cv::Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL:
        QMessageBox::warning(this, "Stitching Error",
            "Camera parameters adjustment failed.");
        setStatus("Fail : camera parameters adjustment impossible.", StatusType::Error);
        break;

    default:
        QMessageBox::warning(this, "Stitching Error", "Unknown error during stitching.");
        setStatus("Fail : unknown error.", StatusType::Error);
        break;
    }
}

void PanoramaStitching::onSave() {
    if (m_result.empty()) {
        QMessageBox::warning(this, "Error", "No result to save.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Save Panorama", "panorama.jpg",
        "Images (*.png *.jpg *.jpeg *.bmp)");
    if (path.isEmpty())
        return;

    if (cv::imwrite(path.toStdString(), m_result))
        setStatus(QString("Panorama saved : %1").arg(path), StatusType::Success);
    else
        QMessageBox::warning(this, "Error", "Impossible to save the image.");
}

void PanoramaStitching::onBack() {
    if (m_startInterface)
        m_startInterface->show();
    close();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void PanoramaStitching::showResultWithFadeIn(const QPixmap& pixmap) {
    ui->resultLabel->setPixmap(pixmap);

    auto* effect = new QGraphicsOpacityEffect(ui->resultLabel);
    ui->resultLabel->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    auto* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(550);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void PanoramaStitching::setStatus(const QString& msg, StatusType type) {
    ui->statusLabel->setText("  " + msg);

    const char* style = nullptr;
    switch (type) {
    case StatusType::Loading:
        style = "color:#7878ff;background:#0a0a22;border:1px solid #252588;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    case StatusType::Success:
        style = "color:#48d890;background:#071510;border:1px solid #1a5c3a;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    case StatusType::Error:
        style = "color:#ff5f5f;background:#180808;border:1px solid #5c1a1a;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    case StatusType::Info:
    default:
        style = "color:#8888aa;background:#0a0a18;border:1px solid #1c1c38;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    }
    ui->statusLabel->setStyleSheet(QString::fromLatin1(style));
}

void PanoramaStitching::refreshThumbnails() {
    ui->imageListWidget->clear();
    for (const QString& path : m_imagePaths) {
        QPixmap thumb(path);
        if (!thumb.isNull())
            thumb = thumb.scaled(110, 82, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        auto* item = new QListWidgetItem(QIcon(thumb), QFileInfo(path).fileName());
        ui->imageListWidget->addItem(item);
    }
}

QPixmap PanoramaStitching::matToPixmap(const cv::Mat& mat) {
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows,
               static_cast<int>(rgb.step), QImage::Format_RGB888);
    return QPixmap::fromImage(img.copy());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stylesheet
// ─────────────────────────────────────────────────────────────────────────────

void PanoramaStitching::applyStyles() {
    setStyleSheet(R"(

/* ── Window & surfaces ───────────────────────────────── */
QMainWindow {
    background-color: #0d0d1c;
}
QWidget#centralwidget {
    background-color: #0d0d1c;
}

/* ── Header ──────────────────────────────────────────── */
QFrame#headerFrame {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #14143a, stop:1 #1e0e38);
    border-bottom: 1px solid #2a2060;
}
QLabel#titleLabel {
    color: #d0d0ff;
    font-family: 'Segoe UI';
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 3px;
    background: transparent;
}
QLabel#subtitleLabel {
    color: #5a5a90;
    font-family: 'Segoe UI';
    font-size: 11px;
    background: transparent;
}

/* ── Footer ──────────────────────────────────────────── */
QFrame#footerFrame {
    background-color: #08080f;
    border-top: 1px solid #181830;
}

/* ── Dividers ─────────────────────────────────────────── */
QFrame#dividerV {
    color: #1e1e40;
    max-width: 1px;
}
QFrame#dividerH {
    color: #1e1e40;
    max-height: 1px;
}

/* ── Section-header labels ───────────────────────────── */
QLabel#labelImages, QLabel#labelMode, QLabel#labelResult {
    color: #50508a;
    font-family: 'Segoe UI';
    font-size: 10px;
    font-weight: bold;
    letter-spacing: 1px;
    background: transparent;
}

/* ── Result display ──────────────────────────────────── */
QLabel#resultLabel {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 10px;
    color: #30304c;
    font-family: 'Segoe UI';
    font-size: 14px;
}

/* ── List widget ─────────────────────────────────────── */
QListWidget#imageListWidget {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 10px;
    color: #c0c0e0;
    font-family: 'Segoe UI';
    font-size: 10px;
    outline: none;
}
QListWidget#imageListWidget::item {
    padding: 4px;
    border-radius: 6px;
    margin: 2px;
    color: #a0a0c8;
}
QListWidget#imageListWidget::item:selected {
    background-color: #28286a;
    color: #ffffff;
    border: 1px solid #4848aa;
}
QListWidget#imageListWidget::item:hover:!selected {
    background-color: #141432;
}

/* ── ComboBox ────────────────────────────────────────── */
QComboBox#modeComboBox {
    background-color: #0a0a1c;
    border: 1px solid #252550;
    border-radius: 8px;
    color: #c0c0e0;
    padding: 4px 10px;
    font-family: 'Segoe UI';
    font-size: 13px;
}
QComboBox#modeComboBox:hover {
    border-color: #4848a8;
    background-color: #0e0e26;
}
QComboBox#modeComboBox::drop-down {
    border: none;
    width: 24px;
}
QComboBox QAbstractItemView {
    background-color: #0d0d1c;
    border: 1px solid #252550;
    color: #c0c0e0;
    selection-background-color: #28286a;
    selection-color: #ffffff;
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

/* ── Add images – green accent ───────────────────────── */
QPushButton#addImagesButton {
    background-color: #0a2518;
    border-color: #185a35;
    color: #48c880;
}
QPushButton#addImagesButton:hover {
    background-color: #103020;
    border-color: #287a50;
    color: #68e8a0;
}
QPushButton#addImagesButton:pressed {
    background-color: #06150e;
    padding-top: 8px;
    padding-bottom: 4px;
}

/* ── Remove – red accent ─────────────────────────────── */
QPushButton#removeImageButton {
    background-color: #250a0a;
    border-color: #5a1818;
    color: #c84848;
}
QPushButton#removeImageButton:hover {
    background-color: #301010;
    border-color: #7a2828;
    color: #e86868;
}
QPushButton#removeImageButton:pressed {
    padding-top: 8px;
    padding-bottom: 4px;
}

/* ── Stitch – gradient accent (main CTA) ─────────────── */
QPushButton#stitchButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff;
    border: none;
    border-radius: 10px;
    font-family: 'Segoe UI';
    font-size: 15px;
    font-weight: bold;
    letter-spacing: 1px;
}
QPushButton#stitchButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#stitchButton:pressed {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #2828c0, stop:1 #6818b0);
    padding-top: 10px;
    padding-bottom: 8px;
}
QPushButton#stitchButton:disabled {
    background: #141428;
    color: #303050;
}

/* ── Save – blue accent ──────────────────────────────── */
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
QPushButton#saveButton:disabled {
    background-color: #0a0a18;
    border-color: #141428;
    color: #283040;
}

/* ── Back – purple-grey ──────────────────────────────── */
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
QScrollBar::handle:vertical:hover {
    background: #3838a0;
}
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
