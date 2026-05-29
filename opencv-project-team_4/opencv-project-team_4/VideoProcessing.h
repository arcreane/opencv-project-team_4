#pragma once
#include <QMainWindow>
#include <QTimer>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class VideoProcessing; }
QT_END_NAMESPACE

class VideoProcessingWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit VideoProcessingWindow(QWidget* startInterface, QWidget* parent = nullptr);
    ~VideoProcessingWindow();

private slots:
    // Tab 1 — Per-Frame Filter
    void onLoadVideoFilter();
    void onFilterSliderChanged(int value);
    void onExportFilter();
    void onExportTimerTick();
    // Tab 2 — Optical Flow
    void onLoadVideoFlow();
    void onToggleFlowPlay();
    void onFlowSpeedChanged(int value);
    void onFlowSliderChanged(int value);
    void onFlowTimerTick();
    // Tab 3 — Motion Recording
    void onStartCam();
    void onStopCam();
    void onCamTimerTick();
    void onMotionThreshChanged(int value);
    // Common
    void onBack();

private:
    enum class StatusType { Info, Success, Error };

    Ui::VideoProcessing* ui;
    QWidget*             m_startInterface;

    // ── Tab 1
    cv::VideoCapture m_filterCap;
    cv::VideoWriter  m_filterWriter;
    QTimer*          m_exportTimer   = nullptr;
    int              m_filterTotal   = 0;
    int              m_exportCurrent = 0;

    // ── Tab 2
    cv::VideoCapture         m_flowCap;
    cv::Mat                  m_prevGrayFlow;
    std::vector<cv::Point2f> m_flowPoints;
    QTimer*                  m_flowTimer    = nullptr;
    int                      m_flowTotal    = 0;
    int                      m_flowFrameIdx = 0;
    bool                     m_flowPlaying  = false;

    // ── Tab 3
    cv::VideoCapture m_webcam;
    cv::VideoWriter  m_motionWriter;
    cv::Mat          m_prevFrameGray;
    QTimer*          m_camTimer        = nullptr;
    bool             m_recording       = false;
    int              m_noMotionFrames  = 0;
    QString          m_motionRecordPath;

    void           applyStyles();
    void           setStatus(const QString& msg, StatusType type = StatusType::Info);
    // Tab 1
    cv::Mat        grabFrame(int index);
    static cv::Mat applyFilter(const cv::Mat& frame, const QString& filterName);
    // Tab 2
    cv::Mat        computeFarnebackFlow(const cv::Mat& prevGray, const cv::Mat& curr);
    cv::Mat        computeLucasKanadeFlow(const cv::Mat& prevGray, const cv::Mat& curr);
    // Shared
    static QPixmap matToPixmap(const cv::Mat& mat);
};
