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
    // Tab 1
    void onLoadVideoFilter();
    void onFilterSliderChanged(int value);
    void onPrevFilterFrame();
    void onNextFilterFrame();
    void onExportFilter();
    void onExportTimerTick();
    // Tab 2
    void onLoadVideoFlow();
    void onToggleFlowPlay();
    void onFlowSpeedChanged(int value);
    void onFlowSliderChanged(int value);
    void onPrevFlowFrame();
    void onNextFlowFrame();
    void onFlowTimerTick();
    // Tab 3
    void onStartCam();
    void onStopCam();
    void onCamTimerTick();
    void onMotionThreshChanged(int value);
    // Tab 4
    void onLoadVideoTimelapse();
    void onTimelapseSliderChanged(int value);
    void onPrevTimelapseFrame();
    void onNextTimelapseFrame();
    void onExportTimelapse();
    void onTimelapseExportTick();
    // Tab 5
    void onLoadVideoStab();
    void onStabilize();
    void onStabSliderChanged(int value);
    void onPrevStabFrame();
    void onNextStabFrame();
    void onExportStab();
    void onStabTimerTick();
    void onStabExportTick();
    // Common
    void onBack();

private:
    enum class StatusType { Info, Success, Error };
    struct StabT { double dx = 0, dy = 0, da = 0; };

    Ui::VideoProcessing* ui;
    QWidget*             m_startInterface;

    // Tab 1
    cv::VideoCapture m_filterCap;
    cv::VideoWriter  m_filterWriter;
    QTimer*          m_exportTimer        = nullptr;
    int              m_filterTotal        = 0;
    int              m_exportCurrent      = 0;
    // Tab 2
    cv::VideoCapture         m_flowCap;
    cv::Mat                  m_prevGrayFlow;
    std::vector<cv::Point2f> m_flowPoints;
    QTimer*                  m_flowTimer    = nullptr;
    int                      m_flowTotal    = 0;
    int                      m_flowFrameIdx = 0;
    bool                     m_flowPlaying  = false;
    // Tab 3
    cv::VideoCapture m_webcam;
    cv::VideoWriter  m_motionWriter;
    cv::Mat          m_prevFrameGray;
    QTimer*          m_camTimer           = nullptr;
    bool             m_recording          = false;
    int              m_noMotionFrames     = 0;
    QString          m_motionRecordPath;
    // Tab 4
    cv::VideoCapture m_timelapseCap;
    cv::VideoWriter  m_timelapseWriter;
    QTimer*          m_timelapseTimer     = nullptr;
    int              m_timelapseTotal     = 0;
    int              m_timelapseExportIdx = 0;
    // Tab 5
    cv::VideoCapture     m_stabCap;
    cv::VideoWriter      m_stabWriter;
    cv::Mat              m_stabPrevGray;
    QTimer*              m_stabTimer       = nullptr;
    QTimer*              m_stabExportTimer = nullptr;
    int                  m_stabTotal       = 0;
    int                  m_stabProcessIdx  = 0;
    int                  m_stabExportIdx   = 0;
    int                  m_stabPhase       = 0;
    std::vector<StabT>   m_stabTransforms;
    std::vector<StabT>   m_stabTrajectory;
    std::vector<StabT>   m_stabSmoothed;

    void           applyStyles();
    void           setStatus(const QString& msg, StatusType type = StatusType::Info);
    static bool    openVideo(cv::VideoCapture& cap, const QString& path);
    cv::Mat        grabFrame(int index);
    cv::Mat        grabTimelapsFrame(int index);
    cv::Mat        computeFarnebackFlow(const cv::Mat& prevGray, const cv::Mat& curr);
    cv::Mat        computeLucasKanadeFlow(const cv::Mat& prevGray, const cv::Mat& curr);
    StabT          computeStabTransform(const cv::Mat& prev, const cv::Mat& curr);
    void           computeTrajectoryAndSmooth();
    cv::Mat        applyStabCorrection(const cv::Mat& frame, int idx);
    static void    showPixmap(QLabel* label, const cv::Mat& mat);
    static QPixmap matToPixmap(const cv::Mat& mat);
    static cv::Mat applyFilter(const cv::Mat& frame, const QString& filterName);
};
