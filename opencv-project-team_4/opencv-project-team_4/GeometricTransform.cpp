#include "GeometricTransform.h"
#include "ui_GeometricTransform.h"
#include <QPainter>

// ============================================================
// ClickableImageLabel
// ============================================================

ClickableImageLabel::ClickableImageLabel(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(400, 300);
}

void ClickableImageLabel::setBasePixmap(const QPixmap& pixmap)
{
    m_basePixmap = pixmap;
    m_points.clear();
    setCursor(Qt::CrossCursor);
    update();
}

void ClickableImageLabel::setMaxPoints(int max)
{
    m_maxPoints = max;
    while (m_points.size() > m_maxPoints)
        m_points.removeLast();
    update();
}

void ClickableImageLabel::clearPoints()
{
    m_points.clear();
    update();
}

QVector<QPointF> ClickableImageLabel::getPoints() const
{
    return m_points;
}

QRectF ClickableImageLabel::imageRect() const
{
    if (m_basePixmap.isNull() || width() == 0 || height() == 0)
        return {};
    QSizeF labelSize(width(), height());
    QSizeF pixSize = QSizeF(m_basePixmap.size()).scaled(labelSize, Qt::KeepAspectRatio);
    qreal x = (labelSize.width()  - pixSize.width())  / 2.0;
    qreal y = (labelSize.height() - pixSize.height()) / 2.0;
    return QRectF(QPointF(x, y), pixSize);
}

QPointF ClickableImageLabel::displayToImage(const QPointF& dp) const
{
    QRectF r = imageRect();
    if (r.isEmpty()) return {};
    return QPointF(
        (dp.x() - r.x()) * m_basePixmap.width()  / r.width(),
        (dp.y() - r.y()) * m_basePixmap.height() / r.height()
    );
}

QPointF ClickableImageLabel::imageToDisplay(const QPointF& ip) const
{
    QRectF r = imageRect();
    if (r.isEmpty()) return {};
    return QPointF(
        ip.x() * r.width()  / m_basePixmap.width()  + r.x(),
        ip.y() * r.height() / m_basePixmap.height() + r.y()
    );
}

void ClickableImageLabel::mousePressEvent(QMouseEvent* event)
{
    if (m_basePixmap.isNull() || event->button() != Qt::LeftButton) return;
    if (m_points.size() >= m_maxPoints) return;

    QPointF pos = event->position();
    QRectF  r   = imageRect();
    if (!r.contains(pos)) return;

    QPointF imgPt = displayToImage(pos);
    imgPt.setX(qBound(0.0, imgPt.x(), (double)(m_basePixmap.width()  - 1)));
    imgPt.setY(qBound(0.0, imgPt.y(), (double)(m_basePixmap.height() - 1)));

    m_points.append(imgPt);
    update();
    emit pointAdded(imgPt);
}

void ClickableImageLabel::paintEvent(QPaintEvent* event)
{
    if (m_basePixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), palette().color(QPalette::Window));
    painter.drawPixmap(imageRect().toRect(), m_basePixmap);

    if (m_points.isEmpty()) return;

    static const QColor kColors[4] = { Qt::red, Qt::green, Qt::blue, Qt::yellow };
    static const QString kLabels[4] = { "P1", "P2", "P3", "P4" };

    painter.setRenderHint(QPainter::Antialiasing);
    for (int i = 0; i < m_points.size(); ++i) {
        QPointF dp = imageToDisplay(m_points[i]);

        painter.setPen(QPen(Qt::black, 1.5));
        painter.setBrush(kColors[i % 4]);
        painter.drawEllipse(dp, 7.0, 7.0);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 8, QFont::Bold));
        painter.drawText(dp + QPointF(9.0, 4.0), kLabels[i]);
    }
}

// ============================================================
// GeometricTransformWindow
// ============================================================

GeometricTransformWindow::GeometricTransformWindow(QWidget* startInterface, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::GeometricTransform)
    , m_startInterface(startInterface)
{
    ui->setupUi(this);
}

GeometricTransformWindow::~GeometricTransformWindow()
{
    delete ui;
}
