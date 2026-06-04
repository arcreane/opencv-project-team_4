#ifndef HISTOGRAMEQUALIZATION_H
#define HISTOGRAMEQUALIZATION_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <opencv2/opencv.hpp>
class StartInterface;
QT_BEGIN_NAMESPACE
namespace Ui { class HistogramEqualizationWindow; }
QT_END_NAMESPACE

class HistogramEqualizationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HistogramEqualizationWindow(QWidget *parent = nullptr);
    ~HistogramEqualizationWindow();

signals:
    void backRequested();

private slots:
    void onLoadImage();
    void onApplyGlobalEqualization();
    void onApplyCLAHE();
    void onResetImage();
    void onSaveResult();
    void onBack();
    void onClipLimitChanged(int value);
    void onTileGridSizeChanged(int value);

private:
    Ui::HistogramEqualizationWindow *ui;
    StartInterface* m_startInterface = nullptr;

    cv::Mat m_originalImage;
    cv::Mat m_resultImage;
    void applyStyles();

    void updatePreview();
    void displayMatOnLabel(const cv::Mat &image, QLabel *label);
    QImage matToQImage(const cv::Mat &image) const;

    cv::Mat applyGlobalEqualization(const cv::Mat &image) const;
    cv::Mat applyCLAHEEqualization(const cv::Mat &image, double clipLimit, int tileGridSize) const;

    bool imageLoaded() const;
    double currentClipLimit() const;
    int currentTileGridSize() const;
};

#endif // HISTOGRAMEQUALIZATION_H
