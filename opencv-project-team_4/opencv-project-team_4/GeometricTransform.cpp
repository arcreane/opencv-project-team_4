#include "GeometricTransform.h"
#include "ui_GeometricTransform.h"
#include <QPainter>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
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
    // Let QLabel draw background, border and placeholder text (from QSS)
    QLabel::paintEvent(event);

    if (m_basePixmap.isNull()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
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
    setAttribute(Qt::WA_DeleteOnClose);

    applyStyles();

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
        setStatus("Erreur : impossible de charger l'image.", StatusType::Error);
        return;
    }

    ui->sourceImageLabel->setBasePixmap(matToPixmap(m_srcImage));

    ui->widthSpinBox->setValue(m_srcImage.cols);
    ui->heightSpinBox->setValue(m_srcImage.rows);
    prefillDestPoints();

    setStatus(QString("Image chargée : %1 × %2 px  —  cliquez sur l'image pour placer les points")
              .arg(m_srcImage.cols).arg(m_srcImage.rows), StatusType::Info);
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
        : "Mode Affine : placez 3 points source sur l'image",
        StatusType::Info);
}

void GeometricTransformWindow::onApplyTransform()
{
    if (m_srcImage.empty()) {
        setStatus("Chargez d'abord une image.", StatusType::Error);
        return;
    }

    bool isPerspective = (ui->transformCombo->currentIndex() == 1);
    int neededPts = isPerspective ? 4 : 3;

    QVector<QPointF> srcPts = ui->sourceImageLabel->getPoints();
    if (srcPts.size() < neededPts) {
        setStatus(QString("Il faut %1 points source (%2 placé(s) actuellement).")
                  .arg(neededPts).arg(srcPts.size()), StatusType::Error);
        return;
    }

    std::vector<cv::Point2f> dst, src;
    for (int i = 0; i < neededPts; ++i) {
        auto* ix = ui->destPointsTable->item(i, 0);
        auto* iy = ui->destPointsTable->item(i, 1);
        if (!ix || !iy || ix->text().isEmpty() || iy->text().isEmpty()) {
            setStatus(QString("Remplissez les coordonnées destination pour P%1.").arg(i + 1),
                      StatusType::Error);
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
        setStatus(QString("Erreur OpenCV : %1").arg(e.what()), StatusType::Error);
        return;
    }

    QSize displaySize = ui->resultLabel->size();
    if (displaySize.width() < 10 || displaySize.height() < 10)
        displaySize = QSize(600, 500);

    showResultWithFadeIn(matToPixmap(m_resultImage)
        .scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->saveButton->setEnabled(true);
    setStatus("Transformation appliquée avec succès.", StatusType::Success);
}

void GeometricTransformWindow::onResetPoints()
{
    ui->sourceImageLabel->clearPoints();
    m_resultImage = cv::Mat();
    ui->resultLabel->clear();
    ui->resultLabel->setText("Placez les points et cliquez sur Appliquer");
    ui->saveButton->setEnabled(false);
    setStatus("Points réinitialisés.", StatusType::Info);
}

void GeometricTransformWindow::onSaveResult()
{
    if (m_resultImage.empty()) return;

    QString path = QFileDialog::getSaveFileName(this, "Sauvegarder l'image", {},
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)");
    if (path.isEmpty()) return;

    if (cv::imwrite(path.toStdString(), m_resultImage))
        setStatus("Image sauvegardée : " + path, StatusType::Success);
    else
        setStatus("Erreur : impossible de sauvegarder l'image.", StatusType::Error);
}

void GeometricTransformWindow::onBack()
{
    if (m_startInterface)
        m_startInterface->show();
    close();
}

void GeometricTransformWindow::setStatus(const QString& msg, StatusType type)
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
    case StatusType::Info:
    default:
        style = "color:#8888aa;background:#0a0a18;border:1px solid #1c1c38;"
                "border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';";
        break;
    }
    ui->statusLabel->setStyleSheet(QString::fromLatin1(style));
}

void GeometricTransformWindow::prefillDestPoints()
{
    int w = ui->widthSpinBox->value();
    int h = ui->heightSpinBox->value();

    const double pts[4][2] = {
        { 0.0,           0.0           },
        { (double)(w-1), 0.0           },
        { 0.0,           (double)(h-1) },
        { (double)(w-1), (double)(h-1) }
    };

    for (int i = 0; i < 4; ++i) {
        ui->destPointsTable->setItem(i, 0, new QTableWidgetItem(QString::number((int)pts[i][0])));
        ui->destPointsTable->setItem(i, 1, new QTableWidgetItem(QString::number((int)pts[i][1])));
    }
}

void GeometricTransformWindow::showResultWithFadeIn(const QPixmap& pixmap)
{
    ui->resultLabel->setPixmap(pixmap);

    auto* effect = new QGraphicsOpacityEffect(ui->resultLabel);
    ui->resultLabel->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    auto* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(550);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
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

// ============================================================
// Stylesheet
// ============================================================

void GeometricTransformWindow::applyStyles()
{
    setStyleSheet(R"(

/* ── Window & surfaces ───────────────────────────────── */
QMainWindow {
    background-color: #0d0d1c;
}
QWidget#centralwidget {
    background-color: #0d0d1c;
}

/* ── Header ──────────────────────────────────────────── */
QFrame#headerFrame {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #14143a, stop:1 #1e0e38);
    border-bottom: 1px solid #2a2060;
}
QLabel#titleLabel {
    color: #d0d0ff;
    font-family: 'Segoe UI';
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 3px;
    background: transparent;
}
QLabel#subtitleLabel {
    color: #5a5a90;
    font-family: 'Segoe UI';
    font-size: 11px;
    background: transparent;
}

/* ── Footer ──────────────────────────────────────────── */
QFrame#footerFrame {
    background-color: #08080f;
    border-top: 1px solid #181830;
}

/* ── Dividers ─────────────────────────────────────────── */
QFrame#divider1, QFrame#divider2 {
    color: #1e1e40;
    max-width: 1px;
}

/* ── Section-header labels ───────────────────────────── */
QLabel#sourceTitleLabel,
QLabel#resultTitleLabel,
QLabel#transformLabel,
QLabel#destLabel,
QLabel#sizeLabel {
    color: #50508a;
    font-family: 'Segoe UI';
    font-size: 10px;
    font-weight: bold;
    letter-spacing: 1px;
    background: transparent;
}

/* ── Hint / size sub-labels ──────────────────────────── */
QLabel#hintLabel {
    color: #38385a;
    font-family: 'Segoe UI';
    font-size: 10px;
    font-style: italic;
    background: transparent;
}
QLabel#widthLabel, QLabel#heightLabel {
    color: #60608a;
    font-family: 'Segoe UI';
    font-size: 12px;
    background: transparent;
}

/* ── Source image label ──────────────────────────────── */
QLabel#sourceImageLabel {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 6px;
    color: #30304c;
    font-family: 'Segoe UI';
    font-size: 14px;
}

/* ── Result display ──────────────────────────────────── */
QLabel#resultLabel {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 10px;
    color: #30304c;
    font-family: 'Segoe UI';
    font-size: 14px;
}

/* ── Table widget ────────────────────────────────────── */
QTableWidget#destPointsTable {
    background-color: #07070f;
    border: 1px solid #1a1a38;
    border-radius: 8px;
    color: #c0c0e0;
    font-family: 'Segoe UI';
    font-size: 12px;
    gridline-color: #181830;
    outline: none;
}
QTableWidget#destPointsTable::item {
    padding: 4px;
}
QTableWidget#destPointsTable::item:selected {
    background-color: #28286a;
    color: #ffffff;
}
QHeaderView::section {
    background-color: #0d0d22;
    color: #6060a0;
    border: none;
    border-bottom: 1px solid #1e1e40;
    font-family: 'Segoe UI';
    font-size: 11px;
    padding: 4px;
}
QTableCornerButton::section {
    background-color: #0d0d22;
    border: none;
}

/* ── SpinBox ─────────────────────────────────────────── */
QSpinBox {
    background-color: #0a0a1c;
    border: 1px solid #252550;
    border-radius: 6px;
    color: #c0c0e0;
    padding: 3px 6px;
    font-family: 'Segoe UI';
    font-size: 14px;
    min-height: 35px;
}
QSpinBox:hover {
    border-color: #4848a8;
    background-color: #0e0e26;
}
QSpinBox::up-button, QSpinBox::down-button {
    background-color: #141432;
    border: none;
    width: 25px;
    height: 16px;
}
QSpinBox::up-button:hover, QSpinBox::down-button:hover {
    background-color: #20204a;
}
QSpinBox::up-arrow {
    image: none;
    width: 0;
    height: 0;
    border-left: 6px solid transparent;
    border-right: 6px solid transparent;
    border-bottom: 8px solid #9090c8;
}
QSpinBox::down-arrow {
    image: none;
    width: 0;
    height: 0;
    border-left: 6px solid transparent;
    border-right: 6px solid transparent;
    border-top: 8px solid #9090c8;
}

/* ── ComboBox ────────────────────────────────────────── */
QComboBox#transformCombo {
    background-color: #0a0a1c;
    border: 1px solid #252550;
    border-radius: 8px;
    color: #c0c0e0;
    padding: 4px 10px;
    font-family: 'Segoe UI';
    font-size: 13px;
}
QComboBox#transformCombo:hover {
    border-color: #4848a8;
    background-color: #0e0e26;
}
QComboBox#transformCombo::drop-down {
    border: none;
    width: 24px;
}
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
    font-family: 'Segoe UI';
    font-size: 12px;
    font-weight: 500;
}
QPushButton:hover {
    background-color: #20204a;
    border-color: #3c3c8a;
    color: #dcdcff;
}
QPushButton:pressed {
    background-color: #0c0c20;
    border-color: #5858b0;
    padding-top: 8px;
    padding-bottom: 4px;
}
QPushButton:disabled {
    background-color: #0c0c18;
    border-color: #141428;
    color: #303050;
}

/* ── Load – green accent ─────────────────────────────── */
QPushButton#loadButton {
    background-color: #0a2518;
    border-color: #185a35;
    color: #48c880;
}
QPushButton#loadButton:hover {
    background-color: #103020;
    border-color: #287a50;
    color: #68e8a0;
}
QPushButton#loadButton:pressed {
    background-color: #06150e;
    padding-top: 8px;
    padding-bottom: 4px;
}

/* ── Apply – gradient accent (main CTA) ──────────────── */
QPushButton#applyButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #3838e8, stop:1 #8820d8);
    color: #ffffff;
    border: none;
    border-radius: 10px;
    font-family: 'Segoe UI';
    font-size: 15px;
    font-weight: bold;
    letter-spacing: 1px;
}
QPushButton#applyButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #5050ff, stop:1 #a030f0);
}
QPushButton#applyButton:pressed {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #2828c0, stop:1 #6818b0);
    padding-top: 10px;
    padding-bottom: 8px;
}
QPushButton#applyButton:disabled {
    background: #141428;
    color: #303050;
}

/* ── Reset – red accent ──────────────────────────────── */
QPushButton#resetButton {
    background-color: #250a0a;
    border-color: #5a1818;
    color: #c84848;
}
QPushButton#resetButton:hover {
    background-color: #301010;
    border-color: #7a2828;
    color: #e86868;
}
QPushButton#resetButton:pressed {
    padding-top: 8px;
    padding-bottom: 4px;
}

/* ── Save – blue accent ──────────────────────────────── */
QPushButton#saveButton {
    background-color: #0a1828;
    border-color: #183868;
    color: #4898d0;
}
QPushButton#saveButton:hover {
    background-color: #101e32;
    border-color: #285898;
    color: #68b8f0;
}
QPushButton#saveButton:disabled {
    background-color: #0a0a18;
    border-color: #141428;
    color: #283040;
}

/* ── Back – purple-grey ──────────────────────────────── */
QPushButton#backButton {
    background-color: #181424;
    border-color: #362850;
    color: #8860b8;
}
QPushButton#backButton:hover {
    background-color: #20182e;
    border-color: #503878;
    color: #a888d8;
}

/* ── Scrollbars ──────────────────────────────────────── */
QScrollBar:vertical {
    background: #07070f;
    width: 6px;
    margin: 0;
    border-radius: 3px;
}
QScrollBar::handle:vertical {
    background: #252558;
    border-radius: 3px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover { background: #3838a0; }
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical { height: 0; }

QScrollBar:horizontal {
    background: #07070f;
    height: 6px;
    border-radius: 3px;
}
QScrollBar::handle:horizontal {
    background: #252558;
    border-radius: 3px;
}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal { width: 0; }

/* ── Menu / status bars ──────────────────────────────── */
QMenuBar {
    background-color: #08080f;
    color: #606090;
    border-bottom: 1px solid #14142a;
}
QMenuBar::item:selected {
    background-color: #1a1a38;
    color: #c0c0e0;
}
QStatusBar {
    background-color: #08080f;
    color: #404070;
    border-top: 1px solid #14142a;
    font-size: 11px;
}

)");
}
