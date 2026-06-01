#pragma once
#include <QMainWindow>
#include <QStringList>
#include <QTimer>
#include <opencv2/core.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class PanoramaStitching; }
QT_END_NAMESPACE

class PanoramaStitching : public QMainWindow {
    Q_OBJECT
public:
    explicit PanoramaStitching(QWidget* startInterface, QWidget* parent = nullptr);
    ~PanoramaStitching();

private slots:
    void onAddImages();
    void onRemoveImage();
    void onStitch();
    void onSave();
    void onBack();

private:
    enum class StatusType { Info, Loading, Success, Error };

    Ui::PanoramaStitching* ui;
    QWidget*    m_startInterface;
    cv::Mat     m_result;
    QStringList m_imagePaths;
    QTimer*     m_loadingTimer  = nullptr;
    int         m_loadingDots   = 0;

    void applyStyles();
    void setStatus(const QString& msg, StatusType type);
    void showResultWithFadeIn(const QPixmap& pixmap);
    void refreshThumbnails();

    static QPixmap matToPixmap(const cv::Mat& mat);
};
