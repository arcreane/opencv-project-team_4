#include "VideoProcessing.h"
#include "ui_VideoProcessing.h"
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video.hpp>

// ============================================================
// Construction
// ============================================================

VideoProcessingWindow::VideoProcessingWindow(QWidget* startInterface, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoProcessing)
    , m_startInterface(startInterface)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    applyStyles();

    // ── Tab 1 timer
    m_exportTimer = new QTimer(this);
    m_exportTimer->setInterval(30);
    connect(m_exportTimer, &QTimer::timeout, this, &VideoProcessingWindow::onExportTimerTick);

    // ── Tab 2 timer
    m_flowTimer = new QTimer(this);
    m_flowTimer->setInterval(60); // default ~16fps
    connect(m_flowTimer, &QTimer::timeout, this, &VideoProcessingWindow::onFlowTimerTick);

    // ── Tab 1 connections
    connect(ui->loadFilterButton,   &QPushButton::clicked,
            this, &VideoProcessingWindow::onLoadVideoFilter);
    connect(ui->filterSlider,       &QSlider::valueChanged,
            this, &VideoProcessingWindow::onFilterSliderChanged);
    connect(ui->filterCombo,        &QComboBox::currentIndexChanged,
            this, [this](int){ onFilterSliderChanged(ui->filterSlider->value()); });
    connect(ui->exportFilterButton, &QPushButton::clicked,
            this, &VideoProcessingWindow::onExportFilter);

    // ── Tab 2 connections
    connect(ui->loadFlowButton,  &QPushButton::clicked,
            this, &VideoProcessingWindow::onLoadVideoFlow);
    connect(ui->playFlowButton,  &QPushButton::clicked,
            this, &VideoProcessingWindow::onToggleFlowPlay);
    connect(ui->flowSpeedSlider, &QSlider::valueChanged,
            this, &VideoProcessingWindow::onFlowSpeedChanged);
    connect(ui->flowSlider,      &QSlider::valueChanged,
            this, &VideoProcessingWindow::onFlowSliderChanged);
    connect(ui->flowCombo,       &QComboBox::currentIndexChanged,
            this, [this](int){ m_flowPoints.clear(); m_prevGrayFlow = cv::Mat(); });

    // ── Common
    connect(ui->backButton, &QPushButton::clicked, this, &VideoProcessingWindow::onBack);
}

VideoProcessingWindow::~VideoProcessingWindow()
{
    m_exportTimer->stop();
    m_flowTimer->stop();
    m_filterWriter.release();
    m_filterCap.release();
    m_flowCap.release();
    delete ui;
}

// ============================================================
// Tab 1 — Per-Frame Filter
// ============================================================

void VideoProcessingWindow::onLoadVideoFilter()
{
    QString path = QFileDialog::getOpenFileName(this, "Ouvrir une vidéo", {},
        "Vidéos (*.mp4 *.avi *.mkv *.mov *.wmv)");
    if (path.isEmpty()) return;

    m_filterCap.release();
    m_filterCap.open(path.toStdString());
    if (!m_filterCap.isOpened()) {
        setStatus("Erreur : impossible d'ouvrir la vidéo.", StatusType::Error);
        return;
    }

    m_filterTotal = (int)m_filterCap.get(cv::CAP_PROP_FRAME_COUNT);
    int fps = (int)m_filterCap.get(cv::CAP_PROP_FPS);
    int w   = (int)m_filterCap.get(cv::CAP_PROP_FRAME_WIDTH);
    int h   = (int)m_filterCap.get(cv::CAP_PROP_FRAME_HEIGHT);

    ui->filterSlider->setRange(0, qMax(0, m_filterTotal - 1));
    ui->filterSlider->setValue(0);
    ui->exportFilterButton->setEnabled(true);

    onFilterSliderChanged(0);
    setStatus(QString("Vidéo chargée : %1×%2  |  %3 fps  |  %4 frames")
              .arg(w).arg(h).arg(fps).arg(m_filterTotal));
}

void VideoProcessingWindow::onFilterSliderChanged(int value)
{
    cv::Mat frame = grabFrame(value);
    if (frame.empty()) return;

    cv::Mat filtered = applyFilter(frame, ui->filterCombo->currentText());
    QPixmap pixmap   = matToPixmap(filtered);
    QSize   sz       = ui->filterPreviewLabel->size();
    ui->filterPreviewLabel->setPixmap(
        sz.width() > 10
        ? pixmap.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        : pixmap);
    ui->frameCountLabel->setText(
        QString("%1 / %2").arg(value).arg(qMax(0, m_filterTotal - 1)));
}

void VideoProcessingWindow::onExportFilter()
{
    if (!m_filterCap.isOpened()) return;

    QString path = QFileDialog::getSaveFileName(this, "Exporter la vidéo", "output.mp4",
        "MP4 (*.mp4);;AVI (*.avi)");
    if (path.isEmpty()) return;

    double fps  = m_filterCap.get(cv::CAP_PROP_FPS);
    int    w    = (int)m_filterCap.get(cv::CAP_PROP_FRAME_WIDTH);
    int    h    = (int)m_filterCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fourcc  = path.endsWith(".mp4", Qt::CaseInsensitive)
                ? cv::VideoWriter::fourcc('m','p','4','v')
                : cv::VideoWriter::fourcc('M','J','P','G');

    if (!m_filterWriter.open(path.toStdString(), fourcc, fps, cv::Size(w, h))) {
        setStatus("Erreur : impossible de créer le fichier de sortie.", StatusType::Error);
        return;
    }

    m_filterCap.set(cv::CAP_PROP_POS_FRAMES, 0);
    m_exportCurrent = 0;
    ui->filterProgressBar->setRange(0, m_filterTotal);
    ui->filterProgressBar->setValue(0);
    ui->filterProgressBar->show();
    ui->exportFilterButton->setEnabled(false);
    m_exportTimer->start();
    setStatus("Export en cours...");
}

void VideoProcessingWindow::onExportTimerTick()
{
    const QString filter    = ui->filterCombo->currentText();
    const int     batchSize = 5;

    for (int i = 0; i < batchSize && m_exportCurrent < m_filterTotal; ++i) {
        cv::Mat frame;
        if (!m_filterCap.read(frame)) { m_exportCurrent = m_filterTotal; break; }
        m_filterWriter.write(applyFilter(frame, filter));
        ++m_exportCurrent;
    }

    ui->filterProgressBar->setValue(m_exportCurrent);

    if (m_exportCurrent >= m_filterTotal) {
        m_exportTimer->stop();
        m_filterWriter.release();
        ui->filterProgressBar->hide();
        ui->exportFilterButton->setEnabled(true);
        setStatus("Export terminé avec succès.", StatusType::Success);
    }
}

// ============================================================
// Tab 2 — Optical Flow
// ============================================================

void VideoProcessingWindow::onLoadVideoFlow()
{
    QString path = QFileDialog::getOpenFileName(this, "Ouvrir une vidéo", {},
        "Vidéos (*.mp4 *.avi *.mkv *.mov *.wmv)");
    if (path.isEmpty()) return;

    m_flowCap.release();
    m_flowCap.open(path.toStdString());
    if (!m_flowCap.isOpened()) {
        setStatus("Erreur : impossible d'ouvrir la vidéo.", StatusType::Error);
        return;
    }

    m_flowTotal    = (int)m_flowCap.get(cv::CAP_PROP_FRAME_COUNT);
    m_flowFrameIdx = 0;
    m_prevGrayFlow = cv::Mat();
    m_flowPoints.clear();

    int fps = (int)m_flowCap.get(cv::CAP_PROP_FPS);
    int w   = (int)m_flowCap.get(cv::CAP_PROP_FRAME_WIDTH);
    int h   = (int)m_flowCap.get(cv::CAP_PROP_FRAME_HEIGHT);

    ui->flowSlider->setRange(0, qMax(0, m_flowTotal - 1));
    ui->flowSlider->blockSignals(true);
    ui->flowSlider->setValue(0);
    ui->flowSlider->blockSignals(false);
    ui->playFlowButton->setEnabled(true);

    // Display first frame
    cv::Mat first;
    m_flowCap.read(first);
    if (!first.empty()) {
        QPixmap px = matToPixmap(first);
        QSize   sz = ui->flowPreviewLabel->size();
        ui->flowPreviewLabel->setPixmap(
            sz.width() > 10
            ? px.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            : px);
    }
    m_flowCap.set(cv::CAP_PROP_POS_FRAMES, 0);

    ui->flowCountLabel->setText(QString("0 / %1").arg(m_flowTotal - 1));
    setStatus(QString("Vidéo chargée : %1×%2  |  %3 fps  |  %4 frames — cliquez Play")
              .arg(w).arg(h).arg(fps).arg(m_flowTotal));
}

void VideoProcessingWindow::onToggleFlowPlay()
{
    if (!m_flowCap.isOpened()) return;

    m_flowPlaying = !m_flowPlaying;
    if (m_flowPlaying) {
        m_flowCap.set(cv::CAP_PROP_POS_FRAMES, (double)m_flowFrameIdx);
        m_prevGrayFlow = cv::Mat();
        m_flowPoints.clear();
        m_flowTimer->start();
        ui->playFlowButton->setText("Pause");
        setStatus("Visualisation Optical Flow en cours...");
    } else {
        m_flowTimer->stop();
        ui->playFlowButton->setText("Play");
        setStatus("Lecture pausée.");
    }
}

void VideoProcessingWindow::onFlowSpeedChanged(int value)
{
    // value 1-10 → interval 200ms-20ms
    int interval = qMax(16, 210 - value * 20);
    m_flowTimer->setInterval(interval);
}

void VideoProcessingWindow::onFlowSliderChanged(int value)
{
    if (!m_flowCap.isOpened()) return;

    if (m_flowPlaying) {
        m_flowTimer->stop();
        m_flowPlaying = false;
        ui->playFlowButton->setText("Play");
    }

    m_flowFrameIdx = value;
    m_prevGrayFlow = cv::Mat();
    m_flowPoints.clear();

    m_flowCap.set(cv::CAP_PROP_POS_FRAMES, (double)value);
    cv::Mat frame;
    m_flowCap.read(frame);
    if (!frame.empty()) {
        QPixmap px = matToPixmap(frame);
        QSize   sz = ui->flowPreviewLabel->size();
        ui->flowPreviewLabel->setPixmap(
            sz.width() > 10
            ? px.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            : px);
    }
    ui->flowCountLabel->setText(
        QString("%1 / %2").arg(value).arg(m_flowTotal - 1));
}

void VideoProcessingWindow::onFlowTimerTick()
{
    cv::Mat currFrame;
    if (!m_flowCap.read(currFrame) || currFrame.empty()) {
        m_flowTimer->stop();
        m_flowPlaying = false;
        ui->playFlowButton->setText("Play");
        m_flowFrameIdx = 0;
        setStatus("Lecture terminée.", StatusType::Success);
        return;
    }

    cv::Mat result;
    if (!m_prevGrayFlow.empty()) {
        if (ui->flowCombo->currentText().contains("Farneback"))
            result = computeFarnebackFlow(m_prevGrayFlow, currFrame);
        else
            result = computeLucasKanadeFlow(m_prevGrayFlow, currFrame);
    } else {
        result = currFrame.clone();
    }

    cv::Mat currGray;
    cv::cvtColor(currFrame, currGray, cv::COLOR_BGR2GRAY);
    m_prevGrayFlow = currGray;
    m_flowFrameIdx++;

    QPixmap px = matToPixmap(result);
    QSize   sz = ui->flowPreviewLabel->size();
    ui->flowPreviewLabel->setPixmap(
        sz.width() > 10
        ? px.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        : px);

    ui->flowSlider->blockSignals(true);
    ui->flowSlider->setValue(m_flowFrameIdx);
    ui->flowSlider->blockSignals(false);
    ui->flowCountLabel->setText(
        QString("%1 / %2").arg(m_flowFrameIdx).arg(m_flowTotal - 1));

    if (m_flowFrameIdx >= m_flowTotal - 1) {
        m_flowTimer->stop();
        m_flowPlaying = false;
        ui->playFlowButton->setText("Play");
        setStatus("Lecture terminée.", StatusType::Success);
    }
}

cv::Mat VideoProcessingWindow::computeFarnebackFlow(const cv::Mat& prevGray, const cv::Mat& curr)
{
    cv::Mat currGray;
    if (curr.channels() == 1)
        currGray = curr;
    else
        cv::cvtColor(curr, currGray, cv::COLOR_BGR2GRAY);

    cv::Mat flow;
    cv::calcOpticalFlowFarneback(prevGray, currGray, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

    std::vector<cv::Mat> parts(2);
    cv::split(flow, parts);

    cv::Mat magnitude, angle, magNorm;
    cv::cartToPolar(parts[0], parts[1], magnitude, angle, true);
    cv::normalize(magnitude, magNorm, 0, 255, cv::NORM_MINMAX);

    cv::Mat hue, sat, val;
    angle.convertTo(hue, CV_8U, 0.5);
    sat = cv::Mat::ones(angle.size(), CV_8U) * 255;
    magNorm.convertTo(val, CV_8U);

    cv::Mat hsv, bgr;
    cv::merge(std::vector<cv::Mat>{hue, sat, val}, hsv);
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
    return bgr;
}

cv::Mat VideoProcessingWindow::computeLucasKanadeFlow(const cv::Mat& prevGray, const cv::Mat& curr)
{
    cv::Mat currGray;
    if (curr.channels() == 1)
        currGray = curr;
    else
        cv::cvtColor(curr, currGray, cv::COLOR_BGR2GRAY);

    cv::Mat drawFrame = curr.channels() == 1
                      ? cv::Mat()
                      : curr.clone();
    if (drawFrame.empty())
        cv::cvtColor(curr, drawFrame, cv::COLOR_GRAY2BGR);

    // Reinitialize feature points when empty
    if (m_flowPoints.empty())
        cv::goodFeaturesToTrack(prevGray, m_flowPoints, 200, 0.01, 10);

    if (m_flowPoints.empty())
        return drawFrame;

    std::vector<cv::Point2f> nextPts;
    std::vector<uchar>       status;
    std::vector<float>       err;
    cv::calcOpticalFlowPyrLK(prevGray, currGray, m_flowPoints, nextPts, status, err,
                              cv::Size(21, 21), 3);

    std::vector<cv::Point2f> goodPts;
    for (size_t i = 0; i < m_flowPoints.size(); ++i) {
        if (!status[i]) continue;
        cv::arrowedLine(drawFrame, m_flowPoints[i], nextPts[i],
                        cv::Scalar(0, 255, 0), 2, cv::LINE_AA, 0, 0.3);
        cv::circle(drawFrame, nextPts[i], 3, cv::Scalar(0, 80, 255), -1);
        goodPts.push_back(nextPts[i]);
    }

    m_flowPoints = goodPts;
    if (m_flowPoints.size() < 20)
        m_flowPoints.clear(); // will reinitialize next tick

    return drawFrame;
}

// ============================================================
// Common
// ============================================================

void VideoProcessingWindow::onBack()
{
    if (m_startInterface)
        m_startInterface->show();
    close();
}

cv::Mat VideoProcessingWindow::grabFrame(int index)
{
    if (!m_filterCap.isOpened()) return {};
    m_filterCap.set(cv::CAP_PROP_POS_FRAMES, (double)index);
    cv::Mat frame;
    m_filterCap.read(frame);
    return frame;
}

QPixmap VideoProcessingWindow::matToPixmap(const cv::Mat& mat)
{
    if (mat.empty()) return {};
    cv::Mat rgb;
    if (mat.channels() == 1)
        cv::cvtColor(mat, rgb, cv::COLOR_GRAY2RGB);
    else
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(
        QImage(rgb.data, rgb.cols, rgb.rows, (int)rgb.step, QImage::Format_RGB888).copy());
}

cv::Mat VideoProcessingWindow::applyFilter(const cv::Mat& frame, const QString& filterName)
{
    if (frame.empty()) return {};
    cv::Mat result;

    if (filterName == "Grayscale") {
        cv::cvtColor(frame, result, cv::COLOR_BGR2GRAY);
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    } else if (filterName == "Blur") {
        cv::GaussianBlur(frame, result, cv::Size(15, 15), 0);
    } else if (filterName == "Canny Edge") {
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, result, 50, 150);
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    } else if (filterName == "Sepia") {
        cv::Mat f;
        frame.convertTo(f, CV_32FC3);
        cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
            0.131f, 0.534f, 0.272f,
            0.168f, 0.686f, 0.349f,
            0.189f, 0.769f, 0.393f);
        cv::transform(f, f, kernel);
        cv::threshold(f, f, 255.0, 255.0, cv::THRESH_TRUNC);
        f.convertTo(result, CV_8UC3);
    } else if (filterName == "Invert") {
        cv::bitwise_not(frame, result);
    } else if (filterName == "Sharpen") {
        cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
             0.f, -1.f,  0.f,
            -1.f,  5.f, -1.f,
             0.f, -1.f,  0.f);
        cv::filter2D(frame, result, -1, kernel);
    } else {
        result = frame.clone();
    }
    return result;
}

void VideoProcessingWindow::setStatus(const QString& msg, StatusType type)
{
    ui->statusLabel->setText("  " + msg);
    const char* style = nullptr;
    switch (type) {
    case StatusType::Success:
        style = "color:#48d890;background:#071510;border:1px solid #1a5c3a;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    case StatusType::Error:
        style = "color:#ff5f5f;background:#180808;border:1px solid #5c1a1a;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    default:
        style = "color:#8888aa;background:#0a0a18;border:1px solid #1c1c38;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    }
    ui->statusLabel->setStyleSheet(QString::fromLatin1(style));
}

// ============================================================
// Stylesheet
// ============================================================

void VideoProcessingWindow::applyStyles()
{
    setStyleSheet(R"(

/* ── Window & surfaces ───────────────────────────────── */
QMainWindow, QWidget#centralwidget { background-color: #0d0d1c; }

/* ── Header ──────────────────────────────────────────── */
QFrame#headerFrame {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #14143a, stop:1 #1e0e38);
    border-bottom: 1px solid #2a2060;
}
QLabel#titleLabel {
    color: #d0d0ff; font-family: 'Segoe UI'; font-size: 18px;
    font-weight: bold; letter-spacing: 3px; background: transparent;
}
QLabel#subtitleLabel {
    color: #5a5a90; font-family: 'Segoe UI'; font-size: 11px;
    background: transparent;
}

/* ── Footer ──────────────────────────────────────────── */
QFrame#footerFrame { background-color: #08080f; border-top: 1px solid #181830; }

/* ── Tab widget ──────────────────────────────────────── */
QTabWidget#tabWidget::pane { border: none; background-color: #0d0d1c; }
QTabWidget#tabWidget > QWidget { background-color: #0d0d1c; }
QTabBar::tab {
    background-color: #0c0c1e; color: #5a5a90;
    border: 1px solid #1a1a38; border-bottom: none;
    padding: 8px 18px; font-family: 'Segoe UI'; font-size: 12px;
}
QTabBar::tab:selected {
    background-color: #161630; color: #c0c0ff;
    border-top: 2px solid #5050d0;
}
QTabBar::tab:hover:!selected { background-color: #12122a; color: #9090d0; }

/* ── Labels ──────────────────────────────────────────── */
QLabel#filterLabel, QLabel#frameLabel,
QLabel#flowMethodLabel, QLabel#flowSpeedLabel, QLabel#flowFrameLabel {
    color: #60608a; font-family: 'Segoe UI'; font-size: 12px;
    background: transparent;
}
QLabel#frameCountLabel, QLabel#flowCountLabel {
    color: #7070a0; font-family: 'Segoe UI'; font-size: 12px;
    background: transparent;
}
QLabel#opticalFlowPlaceholder, QLabel#motionPlaceholder,
QLabel#timelapsePlaceholder, QLabel#stabilizationPlaceholder {
    color: #30304c; font-family: 'Segoe UI'; font-size: 16px;
    background: transparent;
}

/* ── Preview labels ──────────────────────────────────── */
QLabel#filterPreviewLabel, QLabel#flowPreviewLabel {
    background-color: #07070f; border: 1px solid #1a1a38;
    border-radius: 8px; color: #30304c;
    font-family: 'Segoe UI'; font-size: 14px;
}

/* ── Sliders ─────────────────────────────────────────── */
QSlider::groove:horizontal {
    background: #141428; height: 4px; border-radius: 2px;
}
QSlider::handle:horizontal {
    background: #5050c8; width: 14px; height: 14px;
    margin: -5px 0; border-radius: 7px;
}
QSlider::handle:horizontal:hover { background: #7070e8; }
QSlider::sub-page:horizontal { background: #3838a0; border-radius: 2px; }

/* ── Progress bar ────────────────────────────────────── */
QProgressBar#filterProgressBar {
    background-color: #0a0a1c; border: 1px solid #1a1a38;
    border-radius: 4px; text-align: center;
    color: #8888cc; font-size: 11px;
}
QProgressBar#filterProgressBar::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    border-radius: 3px;
}

/* ── ComboBox ────────────────────────────────────────── */
QComboBox {
    background-color: #0a0a1c; border: 1px solid #252550;
    border-radius: 8px; color: #c0c0e0;
    padding: 4px 10px; font-family: 'Segoe UI'; font-size: 13px;
}
QComboBox:hover { border-color: #4848a8; background-color: #0e0e26; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background-color: #0d0d1c; border: 1px solid #252550;
    color: #c0c0e0; selection-background-color: #28286a;
    selection-color: #ffffff;
}

/* ── Buttons – base ──────────────────────────────────── */
QPushButton {
    background-color: #16162e; color: #b0b0d0;
    border: 1px solid #26264c; border-radius: 8px;
    padding: 6px 14px; font-family: 'Segoe UI';
    font-size: 12px; font-weight: 500;
}
QPushButton:hover { background-color: #20204a; border-color: #3c3c8a; color: #dcdcff; }
QPushButton:pressed { background-color: #0c0c20; border-color: #5858b0; }
QPushButton:disabled { background-color: #0c0c18; border-color: #141428; color: #303050; }

/* ── Load buttons – green ────────────────────────────── */
QPushButton#loadFilterButton, QPushButton#loadFlowButton {
    background-color: #0a2518; border-color: #185a35; color: #48c880;
}
QPushButton#loadFilterButton:hover, QPushButton#loadFlowButton:hover {
    background-color: #103020; border-color: #287a50; color: #68e8a0;
}

/* ── Export – gradient CTA ───────────────────────────── */
QPushButton#exportFilterButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff; border: none; border-radius: 8px; font-weight: bold;
}
QPushButton#exportFilterButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#exportFilterButton:disabled { background: #141428; color: #303050; }

/* ── Play – blue accent ──────────────────────────────── */
QPushButton#playFlowButton {
    background-color: #0a1828; border-color: #183868; color: #4898d0;
}
QPushButton#playFlowButton:hover {
    background-color: #101e32; border-color: #285898; color: #68b8f0;
}
QPushButton#playFlowButton:disabled {
    background-color: #0a0a18; border-color: #141428; color: #283040;
}

/* ── Back ────────────────────────────────────────────── */
QPushButton#backButton {
    background-color: #181424; border-color: #362850; color: #8860b8;
}
QPushButton#backButton:hover {
    background-color: #20182e; border-color: #503878; color: #a888d8;
}

/* ── Scrollbars ──────────────────────────────────────── */
QScrollBar:vertical { background: #07070f; width: 6px; border-radius: 3px; }
QScrollBar::handle:vertical { background: #252558; border-radius: 3px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background: #3838a0; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

/* ── MenuBar / StatusBar ─────────────────────────────── */
QMenuBar { background-color: #08080f; color: #606090; border-bottom: 1px solid #14142a; }
QMenuBar::item:selected { background-color: #1a1a38; color: #c0c0e0; }
QStatusBar { background-color: #08080f; color: #404070; font-size: 11px; }

)");
}
