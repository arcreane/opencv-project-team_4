#include "GeometricTransform.h"
#include "ui_GeometricTransform.h"
#include <QPainter>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

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

    // Table setup
    ui->destPointsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->destPointsTable->setRowHidden(3, true); // Affine starts with 3 points

    // Initial source label max points (Affine = 3)
    ui->sourceImageLabel->setMaxPoints(3);

    prefillDestPoints();

    connect(ui->loadButton,     &QPushButton::clicked,
            this, &GeometricTransformWindow::onLoadImage);
    connect(ui->transformCombo, &QComboBox::currentIndexChanged,
            this, &GeometricTransformWindow::onTransformTypeChanged);
    connect(ui->applyButton,    &QPushButton::clicked,
            this, &GeometricTransformWindow::onApplyTransform);
    connect(ui->resetButton,    &QPushButton::clicked,
            this, &GeometricTransformWindow::onResetPoints);
    connect(ui->saveButton,     &QPushButton::clicked,
            this, &GeometricTransformWindow::onSaveResult);
    connect(ui->backButton,     &QPushButton::clicked,
            this, &GeometricTransformWindow::onBack);
}

GeometricTransformWindow::~GeometricTransformWindow()
{
    delete ui;
}

void GeometricTransformWindow::onLoadImage()
{
    QString path = QFileDialog::getOpenFileName(this, "Ouvrir une image", {},
        "Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)");
    if (path.isEmpty()) return;

    m_srcImage = cv::imread(path.toStdString());
    if (m_srcImage.empty()) {
        setStatus("Erreur : impossible de charger l'image.");
        return;
    }

    ui->sourceImageLabel->setBasePixmap(matToPixmap(m_srcImage));

    ui->widthSpinBox->setValue(m_srcImage.cols);
    ui->heightSpinBox->setValue(m_srcImage.rows);
    prefillDestPoints();

    setStatus(QString("Image chargée : %1 × %2 px  —  cliquez sur l'image pour placer les points")
              .arg(m_srcImage.cols).arg(m_srcImage.rows));
}

void GeometricTransformWindow::onTransformTypeChanged(int index)
{
    bool isPerspective = (index == 1);
    int maxPts = isPerspective ? 4 : 3;

    ui->sourceImageLabel->setMaxPoints(maxPts);
    ui->destPointsTable->setRowHidden(3, !isPerspective);
    prefillDestPoints();

    setStatus(isPerspective
        ? "Mode Perspective : placez 4 points source sur l'image"
        : "Mode Affine : placez 3 points source sur l'image");
}

void GeometricTransformWindow::onApplyTransform()
{
    if (m_srcImage.empty()) {
        setStatus("Chargez d'abord une image.");
        return;
    }

    bool isPerspective = (ui->transformCombo->currentIndex() == 1);
    int neededPts = isPerspective ? 4 : 3;

    QVector<QPointF> srcPts = ui->sourceImageLabel->getPoints();
    if (srcPts.size() < neededPts) {
        setStatus(QString("Il faut %1 points source (%2 placé(s) actuellement).")
                  .arg(neededPts).arg(srcPts.size()));
        return;
    }

    // Read destination coordinates from table
    std::vector<cv::Point2f> dst, src;
    for (int i = 0; i < neededPts; ++i) {
        auto* ix = ui->destPointsTable->item(i, 0);
        auto* iy = ui->destPointsTable->item(i, 1);
        if (!ix || !iy || ix->text().isEmpty() || iy->text().isEmpty()) {
            setStatus(QString("Remplissez les coordonnées destination pour P%1.").arg(i + 1));
            return;
        }
        dst.push_back({ ix->text().toFloat(), iy->text().toFloat() });
        src.push_back({ (float)srcPts[i].x(), (float)srcPts[i].y() });
    }

    int outW = ui->widthSpinBox->value();
    int outH = ui->heightSpinBox->value();

    try {
        if (!isPerspective) {
            cv::Mat M = cv::getAffineTransform(src, dst);
            cv::warpAffine(m_srcImage, m_resultImage, M, { outW, outH });
        } else {
            cv::Mat M = cv::getPerspectiveTransform(src, dst);
            cv::warpPerspective(m_srcImage, m_resultImage, M, { outW, outH });
        }
    } catch (const cv::Exception& e) {
        setStatus(QString("Erreur OpenCV : %1").arg(e.what()));
        return;
    }

    QPixmap result = matToPixmap(m_resultImage);
    QSize displaySize = ui->resultLabel->size();
    if (displaySize.width() < 10 || displaySize.height() < 10)
        displaySize = QSize(600, 500);
    ui->resultLabel->setPixmap(result.scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->saveButton->setEnabled(true);
    setStatus("Transformation appliquée avec succès.");
}

void GeometricTransformWindow::onResetPoints()
{
    ui->sourceImageLabel->clearPoints();
    m_resultImage = cv::Mat();
    ui->resultLabel->clear();
    ui->resultLabel->setText("Placez les points et cliquez sur Appliquer");
    ui->saveButton->setEnabled(false);
    setStatus("Points réinitialisés.");
}

void GeometricTransformWindow::onSaveResult()
{
    if (m_resultImage.empty()) return;

    QString path = QFileDialog::getSaveFileName(this, "Sauvegarder l'image", {},
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)");
    if (path.isEmpty()) return;

    if (cv::imwrite(path.toStdString(), m_resultImage))
        setStatus("Image sauvegardée : " + path);
    else
        setStatus("Erreur : impossible de sauvegarder l'image.");
}

void GeometricTransformWindow::onBack()
{
    if (m_startInterface)
        m_startInterface->show();
    close();
}

void GeometricTransformWindow::setStatus(const QString& msg)
{
    ui->statusLabel->setText("  " + msg);
}

void GeometricTransformWindow::prefillDestPoints()
{
    int w = ui->widthSpinBox->value();
    int h = ui->heightSpinBox->value();

    const double pts[4][2] = {
        { 0.0,          0.0         },
        { (double)(w-1), 0.0         },
        { 0.0,          (double)(h-1) },
        { (double)(w-1), (double)(h-1) }
    };

    for (int i = 0; i < 4; ++i) {
        ui->destPointsTable->setItem(i, 0, new QTableWidgetItem(QString::number((int)pts[i][0])));
        ui->destPointsTable->setItem(i, 1, new QTableWidgetItem(QString::number((int)pts[i][1])));
    }
}

QPixmap GeometricTransformWindow::matToPixmap(const cv::Mat& mat)
{
    if (mat.empty()) return {};
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(
        QImage(rgb.data, rgb.cols, rgb.rows, (int)rgb.step, QImage::Format_RGB888).copy()
    );
}
