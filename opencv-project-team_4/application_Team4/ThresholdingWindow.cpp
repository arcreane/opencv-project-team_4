#include "ThresholdingWindow.h"
#include "StartInterface.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSizePolicy>

static QLabel* addSliderRowT(QFormLayout* form, const QString& name, QSlider* slider, const QString& suffix = "")
{
    auto* lbl = new QLabel(QString::number(slider->value()) + suffix);
    lbl->setObjectName("valueLabel");
    lbl->setMinimumWidth(40);
    auto* row = new QHBoxLayout;
    row->addWidget(slider);
    row->addWidget(lbl);
    form->addRow(new QLabel(name + ":"), row);
    QObject::connect(slider, &QSlider::valueChanged, lbl,
        [lbl, suffix](int v) { lbl->setText(QString::number(v) + suffix); });
    return lbl;
}

ThresholdingWindow::ThresholdingWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_startInterface(qobject_cast<StartInterface*>(parent))
{
    setWindowTitle("Thresholding");
    setMinimumSize(1100, 720);
    resize(1280, 800);

    setStyleSheet(
        "QMainWindow,QWidget{background:#1e1e2e;color:#cdd6f4;}"
        "QMenuBar{background:#181825;color:#cdd6f4;padding:2px;}"
        "QMenuBar::item:selected{background:#313244;border-radius:4px;}"
        "QMenu{background:#181825;color:#cdd6f4;border:1px solid #45475a;}"
        "QMenu::item:selected{background:#313244;}"
        "QToolBar{background:#181825;border-bottom:1px solid #45475a;padding:4px;spacing:6px;}"
        "QToolButton{background:#313244;color:#cdd6f4;border-radius:6px;padding:6px 14px;font-weight:bold;font-size:12px;}"
        "QToolButton:hover{background:#45475a;}"
        "QToolButton:pressed{background:#585b70;}"
        "QGroupBox{border:1px solid #45475a;border-radius:6px;margin-top:10px;padding-top:8px;font-weight:bold;color:#89b4fa;}"
        "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 4px;}"
        "QComboBox{background:#313244;border:1px solid #45475a;border-radius:4px;padding:4px 8px;color:#cdd6f4;min-height:24px;}"
        "QComboBox QAbstractItemView{background:#1e1e2e;color:#cdd6f4;selection-background-color:#45475a;}"
        "QCheckBox{color:#cdd6f4;spacing:6px;}"
        "QCheckBox::indicator{width:16px;height:16px;border-radius:3px;border:1px solid #45475a;background:#313244;}"
        "QCheckBox::indicator:checked{background:#89b4fa;border-color:#89b4fa;}"
        "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;border-radius:6px;padding:8px 14px;font-weight:bold;font-size:12px;}"
        "QPushButton:hover{background:#b4d0fa;}"
        "QPushButton:pressed{background:#74a8d8;}"
        "QScrollArea{border:none;background:#11111b;}"
        "QStatusBar{background:#181825;color:#a6adc8;font-size:11px;border-top:1px solid #45475a;}"
        "QLabel#valueLabel{color:#89b4fa;font-size:11px;min-width:36px;}"
    );

    setupUI();
}

ThresholdingWindow::~ThresholdingWindow() {}

void ThresholdingWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (!currentImage.isNull()) showImage(currentImage);
}

QSlider* ThresholdingWindow::makeSlider(int min, int max, int val, Qt::Orientation orient)
{
    auto* s = new QSlider(orient);
    s->setRange(min, max);
    s->setValue(val);
    s->setStyleSheet(
        "QSlider::groove:horizontal{background:#45475a;height:4px;border-radius:2px;}"
        "QSlider::handle:horizontal{background:#89b4fa;width:14px;height:14px;margin:-5px 0;border-radius:7px;}"
        "QSlider::sub-page:horizontal{background:#89b4fa;height:4px;border-radius:2px;}"
    );
    return s;
}

void ThresholdingWindow::setupUI()
{
    setupMenuBar();
    setupToolBar();

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet("QSplitter::handle{background:#45475a;}");

    // Left panel
    auto* leftPanel = new QWidget;
    leftPanel->setMinimumWidth(270);
    leftPanel->setMaximumWidth(320);
    leftPanel->setStyleSheet("background:#181825;");
    auto* leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(8, 8, 8, 8);
    leftLay->setSpacing(8);

    auto* grpMode = new QGroupBox("Thresholding Mode");
    auto* mLay = new QVBoxLayout(grpMode);
    comboThreshMode = new QComboBox;
    comboThreshMode->addItems({ "Binary","Binary Inv","Trunc","ToZero","ToZero Inv","Otsu","Adaptive Gauss","Adaptive Mean" });
    mLay->addWidget(comboThreshMode);
    leftLay->addWidget(grpMode);

    auto* grpParams = new QGroupBox("Parameters");
    auto* pLay = new QFormLayout(grpParams);
    sliderThresh = makeSlider(0, 255, 127);
    addSliderRowT(pLay, "Threshold", sliderThresh);
    sliderBlock = makeSlider(3, 51, 11);
    addSliderRowT(pLay, "Block size", sliderBlock);
    sliderConstC = makeSlider(0, 20, 2);
    addSliderRowT(pLay, "Constant C", sliderConstC);
    leftLay->addWidget(grpParams);

    checkInvert = new QCheckBox("Invert Result");
    leftLay->addWidget(checkInvert);

    auto* btnApply = new QPushButton("Apply");
    leftLay->addWidget(btnApply);
    leftLay->addStretch();

    auto* btnBack = new QPushButton("Back");
    btnBack->setStyleSheet("QPushButton{background:#313244;color:#cdd6f4;}QPushButton:hover{background:#45475a;}");
    leftLay->addWidget(btnBack);

    // Right panel
    auto* rightPanel = new QWidget;
    setupImageCanvas();
    auto* rightLay = new QVBoxLayout(rightPanel);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->addWidget(scrollArea);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    mainLay->addWidget(splitter);

    setupStatusBar();

    connect(comboThreshMode, &QComboBox::currentIndexChanged, this, [this](int idx) { onThreshModeChanged(idx); applyThreshold(); });
    connect(sliderThresh, &QSlider::valueChanged, this, &ThresholdingWindow::applyThreshold);
    connect(sliderBlock, &QSlider::valueChanged, this, &ThresholdingWindow::applyThreshold);
    connect(sliderConstC, &QSlider::valueChanged, this, &ThresholdingWindow::applyThreshold);
    connect(checkInvert, &QCheckBox::stateChanged, this, &ThresholdingWindow::applyThreshold);
    connect(btnApply, &QPushButton::clicked, this, &ThresholdingWindow::applyThreshold);
    connect(btnBack, &QPushButton::clicked, this, &ThresholdingWindow::onBack);

    onThreshModeChanged(0);
}

void ThresholdingWindow::setupMenuBar()
{
    auto* mb = menuBar();
    auto* fm = mb->addMenu("&File");

    actOpen = new QAction("&Open…", this);
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &ThresholdingWindow::openImage);
    fm->addAction(actOpen);

    actSave = new QAction("&Save…", this);
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave, &QAction::triggered, this, &ThresholdingWindow::saveImage);
    fm->addAction(actSave);

    fm->addSeparator();
    auto* actQuit = new QAction("&Quit", this);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actQuit, &QAction::triggered, qApp, &QApplication::quit);
    fm->addAction(actQuit);

    auto* em = mb->addMenu("&Edit");
    actUndo = new QAction("&Undo", this);
    actUndo->setShortcut(QKeySequence::Undo);
    connect(actUndo, &QAction::triggered, this, &ThresholdingWindow::undoAction);
    em->addAction(actUndo);

    actRedo = new QAction("&Redo", this);
    actRedo->setShortcut(QKeySequence::Redo);
    connect(actRedo, &QAction::triggered, this, &ThresholdingWindow::redoAction);
    em->addAction(actRedo);

    em->addSeparator();
    actReset = new QAction("&Reset", this);
    connect(actReset, &QAction::triggered, this, &ThresholdingWindow::resetImage);
    em->addAction(actReset);

    actSave->setEnabled(false);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);
    actReset->setEnabled(false);
}

void ThresholdingWindow::setupToolBar()
{
    auto* tb = addToolBar("Toolbar");
    tb->setMovable(false);
    tb->addAction(actOpen);
    tb->addAction(actSave);
    tb->addSeparator();
    tb->addAction(actUndo);
    tb->addAction(actRedo);
    tb->addSeparator();
    tb->addAction(actReset);
}

void ThresholdingWindow::setupImageCanvas()
{
    scrollArea = new QScrollArea;
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setStyleSheet("background:#11111b;border:none;");

    imageLabel = new QLabel;
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setText(
        "<div style='color:#585b70;text-align:center;'>"
        "<p style='font-size:48px;margin:0;'>🖼</p>"
        "<p style='font-size:18px;margin:8px 0 4px;'>Open an image to start</p>"
        "<p style='font-size:13px;color:#45475a;'>File → Open (Ctrl+O)</p>"
        "</div>"
    );
    imageLabel->setTextFormat(Qt::RichText);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
}

void ThresholdingWindow::setupStatusBar()
{
    statusLabel = new QLabel("No image loaded");
    statusBar()->addWidget(statusLabel, 1);
}

void ThresholdingWindow::showImage(const QImage& img)
{
    imageLabel->setTextFormat(Qt::PlainText);
    imageLabel->setText("");
    QSize available = scrollArea->viewport()->size();
    QPixmap pix = QPixmap::fromImage(img);
    QSize target = pix.size().scaled(available, Qt::KeepAspectRatio);
    if (target.width() > pix.width()) target = pix.size();
    imageLabel->setPixmap(pix.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imageLabel->resize(target);
}

void ThresholdingWindow::updateStatusBar()
{
    if (originalImage.isNull()) { statusLabel->setText("No image loaded"); return; }
    QString s = QString("%1 × %2 px").arg(originalImage.width()).arg(originalImage.height());
    if (!currentFilePath.isEmpty())
        s += "  ·  " + currentFilePath.section('\\', -1).section('/', -1);
    s += "  ·  Undo: " + QString::number(undoStack.size());
    statusLabel->setText(s);
}

void ThresholdingWindow::pushUndo()
{
    if (originalImage.isNull()) return;
    undoStack.push(currentImage.copy());
    if (undoStack.size() > 20) undoStack.removeFirst();
    redoStack.clear();
    actUndo->setEnabled(true);
    actRedo->setEnabled(false);
    updateStatusBar();
}

void ThresholdingWindow::openImage()
{
    QString path = QFileDialog::getOpenFileName(this, "Load an image", "",
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff);;All files (*)");
    if (path.isEmpty()) return;
    QImage img(path);
    if (img.isNull()) { QMessageBox::warning(this, "Error", "Unable to load image."); return; }
    currentFilePath = path;
    img = img.convertToFormat(QImage::Format_RGB888);
    originalImage = img.copy();
    currentImage = img.copy();
    undoStack.clear(); redoStack.clear();
    actSave->setEnabled(true); actReset->setEnabled(true);
    actUndo->setEnabled(false); actRedo->setEnabled(false);
    showImage(currentImage);
    updateStatusBar();
    applyThreshold();
}

void ThresholdingWindow::saveImage()
{
    if (currentImage.isNull()) return;
    QString path = QFileDialog::getSaveFileName(this, "Save Image", "",
        "JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)");
    if (path.isEmpty()) return;
    if (currentImage.save(path)) statusLabel->setText("Saved: " + path);
    else QMessageBox::warning(this, "Error", "Unable to save image.");
}

void ThresholdingWindow::undoAction()
{
    if (undoStack.isEmpty()) return;
    redoStack.push(currentImage.copy());
    currentImage = undoStack.pop();
    showImage(currentImage);
    actUndo->setEnabled(!undoStack.isEmpty());
    actRedo->setEnabled(true);
    updateStatusBar();
}

void ThresholdingWindow::redoAction()
{
    if (redoStack.isEmpty()) return;
    undoStack.push(currentImage.copy());
    currentImage = redoStack.pop();
    showImage(currentImage);
    actRedo->setEnabled(!redoStack.isEmpty());
    actUndo->setEnabled(true);
    updateStatusBar();
}

void ThresholdingWindow::resetImage()
{
    if (originalImage.isNull()) return;
    pushUndo();
    currentImage = originalImage.copy();
    showImage(currentImage);
    updateStatusBar();
}

void ThresholdingWindow::onBack()
{
    if (m_startInterface) m_startInterface->show();
    close();
}

void ThresholdingWindow::onThreshModeChanged(int idx)
{
    bool adaptive = (idx == 6 || idx == 7);
    sliderThresh->setEnabled(idx < 5);
    sliderBlock->setEnabled(adaptive);
    sliderConstC->setEnabled(adaptive);
}

void ThresholdingWindow::applyThreshold()
{
    if (originalImage.isNull()) return;

    QImage gray = originalImage.convertToFormat(QImage::Format_Grayscale8);
    int mode = comboThreshMode->currentIndex();
    int thresh = sliderThresh->value();
    bool inv = checkInvert->isChecked();
    QImage result(gray.size(), QImage::Format_Grayscale8);

    if (mode == 0 || mode == 1) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar* s = gray.constScanLine(y); uchar* d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++)
                d[x] = ((s[x] > thresh) ^ (mode == 1)) ? 255 : 0;
        }
    }
    else if (mode == 2) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar* s = gray.constScanLine(y); uchar* d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] > thresh ? (uchar)thresh : s[x];
        }
    }
    else if (mode == 3) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar* s = gray.constScanLine(y); uchar* d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] > thresh ? s[x] : 0;
        }
    }
    else if (mode == 4) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar* s = gray.constScanLine(y); uchar* d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] <= thresh ? s[x] : 0;
        }
    }
    else if (mode == 5) {
        long long hist[256] = {};
        for (int y = 0; y < gray.height(); y++) {
            const uchar* s = gray.constScanLine(y);
            for (int x = 0; x < gray.width(); x++) hist[s[x]]++;
        }
        long long total = (long long)gray.width() * gray.height();
        double sum = 0;
        for (int i = 0; i < 256; i++) sum += i * hist[i];
        double sumB = 0, wB = 0, maxVar = 0; int otsuT = 0;
        for (int i = 0; i < 256; i++) {
            wB += hist[i]; if (!wB) continue;
            double wF = total - wB; if (!wF) break;
            sumB += i * hist[i];
            double mB = sumB / wB, mF = (sum - sumB) / wF;
            double var = wB * wF * (mB - mF) * (mB - mF);
            if (var > maxVar) { maxVar = var; otsuT = i; }
        }
        for (int y = 0; y < gray.height(); y++) {
            const uchar* s = gray.constScanLine(y); uchar* d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] > otsuT ? 255 : 0;
        }
    }
    else {
        int bs = sliderBlock->value() | 1;
        bs = qMax(3, bs);
        int C = sliderConstC->value(), half = bs / 2;
        for (int y = 0; y < gray.height(); y++) {
            const uchar* src = gray.constScanLine(y);
            uchar* dst = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) {
                long long s2 = 0, cnt = 0;
                for (int dy = -half; dy <= half; dy++) {
                    const uchar* row = gray.constScanLine(qBound(0, y + dy, gray.height() - 1));
                    for (int dx = -half; dx <= half; dx++) {
                        s2 += row[qBound(0, x + dx, gray.width() - 1)]; cnt++;
                    }
                }
                dst[x] = src[x] > (s2 / cnt - C) ? 255 : 0;
            }
        }
    }

    if (inv) result.invertPixels();
    currentImage = result.convertToFormat(QImage::Format_RGB888);
    showImage(currentImage);
}