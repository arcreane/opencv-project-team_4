#pragma once
#include <QMainWindow>
#include <QTimer>
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
    void onLoadVideoFilter();
    void onFilterSliderChanged(int value);
    void onExportFilter();
    void onExportTimerTick();
    void onBack();

private:
    enum class StatusType { Info, Success, Error };

    Ui::VideoProcessing* ui;
    QWidget*             m_startInterface;

    // Tab 1 — Per-Frame Filter
    cv::VideoCapture m_filterCap;
    cv::VideoWriter  m_filterWriter;
    QTimer*          m_exportTimer   = nullptr;
    int              m_filterTotal   = 0;
    int              m_exportCurrent = 0;

    void           applyStyles();
    void           setStatus(const QString& msg, StatusType type = StatusType::Info);
    cv::Mat        grabFrame(int index);
    static QPixmap matToPixmap(const cv::Mat& mat);
    static cv::Mat applyFilter(const cv::Mat& frame, const QString& filterName);
};
