#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QVector>
#include <QPointF>
#include <QPixmap>
#include <QMouseEvent>
#include <opencv2/core.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class GeometricTransform; }
QT_END_NAMESPACE

class ClickableImageLabel : public QLabel {
    Q_OBJECT
public:
    explicit ClickableImageLabel(QWidget* parent = nullptr);

    void             setBasePixmap(const QPixmap& pixmap);
    void             setMaxPoints(int max);
    void             clearPoints();
    QVector<QPointF> getPoints() const;

signals:
    void pointAdded(QPointF imagePoint);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap          m_basePixmap;
    QVector<QPointF> m_points;
    int              m_maxPoints = 4;

    QRectF  imageRect()                        const;
    QPointF displayToImage(const QPointF& dp) const;
    QPointF imageToDisplay(const QPointF& ip) const;
};

class GeometricTransformWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GeometricTransformWindow(QWidget* startInterface, QWidget* parent = nullptr);
    ~GeometricTransformWindow();

private slots:
    void onLoadImage();
    void onTransformTypeChanged(int index);
    void onApplyTransform();
    void onResetPoints();
    void onSaveResult();
    void onBack();

private:
    enum class StatusType { Info, Success, Error };

    Ui::GeometricTransform* ui;
    QWidget*                m_startInterface;
    cv::Mat                 m_srcImage;
    cv::Mat                 m_resultImage;

    void           setStatus(const QString& msg, StatusType type = StatusType::Info);
    void           prefillDestPoints();
    void           applyStyles();
    void           showResultWithFadeIn(const QPixmap& pixmap);
    static QPixmap matToPixmap(const cv::Mat& mat);
};
