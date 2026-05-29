#include "VideoProcessing.h"
#include "ui_VideoProcessing.h"
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

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

    m_exportTimer = new QTimer(this);
    m_exportTimer->setInterval(30);
    connect(m_exportTimer, &QTimer::timeout, this, &VideoProcessingWindow::onExportTimerTick);

    connect(ui->loadFilterButton,  &QPushButton::clicked,
            this, &VideoProcessingWindow::onLoadVideoFilter);
    connect(ui->filterSlider,      &QSlider::valueChanged,
            this, &VideoProcessingWindow::onFilterSliderChanged);
    connect(ui->filterCombo,       &QComboBox::currentIndexChanged,
            this, [this](int){ onFilterSliderChanged(ui->filterSlider->value()); });
    connect(ui->exportFilterButton, &QPushButton::clicked,
            this, &VideoProcessingWindow::onExportFilter);
    connect(ui->backButton,        &QPushButton::clicked,
            this, &VideoProcessingWindow::onBack);
}

VideoProcessingWindow::~VideoProcessingWindow()
{
    m_exportTimer->stop();
    m_filterWriter.release();
    m_filterCap.release();
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
    int fps       = (int)m_filterCap.get(cv::CAP_PROP_FPS);
    int w         = (int)m_filterCap.get(cv::CAP_PROP_FRAME_WIDTH);
    int h         = (int)m_filterCap.get(cv::CAP_PROP_FRAME_HEIGHT);

    ui->filterSlider->setRange(0, qMax(0, m_filterTotal - 1));
    ui->filterSlider->setValue(0);
    ui->exportFilterButton->setEnabled(true);

    onFilterSliderChanged(0);

    setStatus(QString("Vidéo chargée : %1 × %2  |  %3 fps  |  %4 frames")
              .arg(w).arg(h).arg(fps).arg(m_filterTotal), StatusType::Info);
}

void VideoProcessingWindow::onFilterSliderChanged(int value)
{
    cv::Mat frame = grabFrame(value);
    if (frame.empty()) return;

    cv::Mat filtered = applyFilter(frame, ui->filterCombo->currentText());
    QPixmap pixmap   = matToPixmap(filtered);

    QSize labelSize = ui->filterPreviewLabel->size();
    if (labelSize.width() > 10 && labelSize.height() > 10)
        ui->filterPreviewLabel->setPixmap(
            pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        ui->filterPreviewLabel->setPixmap(pixmap);

    ui->frameCountLabel->setText(
        QString("%1 / %2").arg(value).arg(qMax(0, m_filterTotal - 1)));
}

void VideoProcessingWindow::onExportFilter()
{
    if (!m_filterCap.isOpened()) return;

    QString path = QFileDialog::getSaveFileName(this, "Exporter la vidéo", "output.mp4",
        "MP4 (*.mp4);;AVI (*.avi)");
    if (path.isEmpty()) return;

    double fps = m_filterCap.get(cv::CAP_PROP_FPS);
    int    w   = (int)m_filterCap.get(cv::CAP_PROP_FRAME_WIDTH);
    int    h   = (int)m_filterCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fourcc = path.endsWith(".mp4", Qt::CaseInsensitive)
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
    setStatus("Export en cours...", StatusType::Info);
}

void VideoProcessingWindow::onExportTimerTick()
{
    const QString filterName = ui->filterCombo->currentText();
    const int batchSize = 5;

    for (int i = 0; i < batchSize && m_exportCurrent < m_filterTotal; ++i) {
        cv::Mat frame;
        if (!m_filterCap.read(frame)) {
            m_exportCurrent = m_filterTotal; // force end
            break;
        }
        m_filterWriter.write(applyFilter(frame, filterName));
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

void VideoProcessingWindow::onBack()
{
    if (m_startInterface)
        m_startInterface->show();
    close();
}

// ============================================================
// Helpers
// ============================================================

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
        // Matrix rows = [B_out, G_out, R_out], columns = [B_in, G_in, R_in]
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
    color: #d0d0ff;
    font-family: 'Segoe UI'; font-size: 18px;
    font-weight: bold; letter-spacing: 3px;
    background: transparent;
}
QLabel#subtitleLabel {
    color: #5a5a90;
    font-family: 'Segoe UI'; font-size: 11px;
    background: transparent;
}

/* ── Footer ──────────────────────────────────────────── */
QFrame#footerFrame {
    background-color: #08080f;
    border-top: 1px solid #181830;
}

/* ── Tab widget ──────────────────────────────────────── */
QTabWidget#tabWidget::pane {
    border: none;
    background-color: #0d0d1c;
}
QTabWidget#tabWidget > QWidget {
    background-color: #0d0d1c;
}
QTabBar::tab {
    background-color: #0c0c1e;
    color: #5a5a90;
    border: 1px solid #1a1a38;
    border-bottom: none;
    padding: 8px 18px;
    font-family: 'Segoe UI'; font-size: 12px;
}
QTabBar::tab:selected {
    background-color: #161630;
    color: #c0c0ff;
    border-top: 2px solid #5050d0;
}
QTabBar::tab:hover:!selected {
    background-color: #12122a;
    color: #9090d0;
}

/* ── Section labels ──────────────────────────────────── */
QLabel#filterLabel, QLabel#frameLabel {
    color: #60608a;
    font-family: 'Segoe UI'; font-size: 12px;
    background: transparent;
}
QLabel#frameCountLabel {
    color: #7070a0;
    font-family: 'Segoe UI'; font-size: 12px;
    background: transparent;
}

/* ── Placeholder labels ──────────────────────────────── */
QLabel#opticalFlowPlaceholder,
QLabel#motionPlaceholder,
QLabel#timelapsePlaceholder,
QLabel#stabilizationPlaceholder {
    color: #30304c;
    font-family: 'Segoe UI'; font-size: 16px;
    background: transparent;
}

/* ── Preview label ───────────────────────────────────── */
QLabel#filterPreviewLabel {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 8px;
    color: #30304c;
    font-family: 'Segoe UI'; font-size: 14px;
}

/* ── Slider ──────────────────────────────────────────── */
QSlider#filterSlider::groove:horizontal {
    background: #141428;
    height: 4px;
    border-radius: 2px;
}
QSlider#filterSlider::handle:horizontal {
    background: #5050c8;
    width: 14px; height: 14px;
    margin: -5px 0;
    border-radius: 7px;
}
QSlider#filterSlider::handle:horizontal:hover { background: #7070e8; }
QSlider#filterSlider::sub-page:horizontal {
    background: #3838a0;
    border-radius: 2px;
}

/* ── Progress bar ────────────────────────────────────── */
QProgressBar#filterProgressBar {
    background-color: #0a0a1c;
    border: 1px solid #1a1a38;
    border-radius: 4px;
    text-align: center;
    color: #8888cc;
    font-size: 11px;
}
QProgressBar#filterProgressBar::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    border-radius: 3px;
}

/* ── ComboBox ────────────────────────────────────────── */
QComboBox {
    background-color: #0a0a1c;
    border: 1px solid #252550;
    border-radius: 8px;
    color: #c0c0e0;
    padding: 4px 10px;
    font-family: 'Segoe UI'; font-size: 13px;
}
QComboBox:hover { border-color: #4848a8; background-color: #0e0e26; }
QComboBox::drop-down { border: none; width: 24px; }
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
    font-family: 'Segoe UI'; font-size: 12px; font-weight: 500;
}
QPushButton:hover { background-color: #20204a; border-color: #3c3c8a; color: #dcdcff; }
QPushButton:pressed { background-color: #0c0c20; border-color: #5858b0; }
QPushButton:disabled { background-color: #0c0c18; border-color: #141428; color: #303050; }

/* ── Load – green ────────────────────────────────────── */
QPushButton#loadFilterButton {
    background-color: #0a2518; border-color: #185a35; color: #48c880;
}
QPushButton#loadFilterButton:hover {
    background-color: #103020; border-color: #287a50; color: #68e8a0;
}

/* ── Export – gradient CTA ───────────────────────────── */
QPushButton#exportFilterButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff; border: none; border-radius: 8px;
    font-weight: bold;
}
QPushButton#exportFilterButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#exportFilterButton:disabled { background: #141428; color: #303050; }

/* ── Back ────────────────────────────────────────────── */
QPushButton#backButton {
    background-color: #181424; border-color: #362850; color: #8860b8;
}
QPushButton#backButton:hover {
    background-color: #20182e; border-color: #503878; color: #a888d8;
}

/* ── Scrollbars ──────────────────────────────────────── */
QScrollBar:vertical {
    background: #07070f; width: 6px; border-radius: 3px;
}
QScrollBar::handle:vertical { background: #252558; border-radius: 3px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background: #3838a0; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

/* ── MenuBar / StatusBar ─────────────────────────────── */
QMenuBar { background-color: #08080f; color: #606090; border-bottom: 1px solid #14142a; }
QMenuBar::item:selected { background-color: #1a1a38; color: #c0c0e0; }
QStatusBar { background-color: #08080f; color: #404070; font-size: 11px; }

)");
}
