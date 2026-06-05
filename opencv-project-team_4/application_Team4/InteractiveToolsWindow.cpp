#include "InteractiveToolsWindow.h"
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

static QLabel* addSliderRowI(QFormLayout* form, const QString& name, QSlider* slider, const QString& suffix = "")
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

InteractiveToolsWindow::InteractiveToolsWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_startInterface(qobject_cast<StartInterface*>(parent))
{
    setWindowTitle("Interactive Tools");
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
        "QGroupBox{border:1px solid #45475a;border-radius:6px;margin-top:10px;padding-top:8px;font-weight:bold;color:#89b4fa;}"
        "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 4px;}"
        "QComboBox{background:#313244;border:1px solid #45475a;border-radius:4px;padding:4px 8px;color:#cdd6f4;min-height:24px;}"
        "QComboBox QAbstractItemView{background:#1e1e2e;color:#cdd6f4;selection-background-color:#45475a;}"
        "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;border-radius:6px;padding:8px 14px;font-weight:bold;font-size:12px;}"
        "QPushButton:hover{background:#b4d0fa;}"
        "QPushButton:pressed{background:#74a8d8;}"
        "QScrollArea{border:none;background:#11111b;}"
        "QStatusBar{background:#181825;color:#a6adc8;font-size:11px;border-top:1px solid #45475a;}"
        "QLabel#valueLabel{color:#89b4fa;font-size:11px;min-width:36px;}"
    );

    setupUI();
}

InteractiveToolsWindow::~InteractiveToolsWindow() {}

void InteractiveToolsWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (!currentImage.isNull()) showImage(currentImage);
}

QSlider* InteractiveToolsWindow::makeSlider(int min, int max, int val, Qt::Orientation orient)
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

void InteractiveToolsWindow::setupUI()
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

    auto* leftPanel = new QWidget;
    leftPanel->setMinimumWidth(270);
    leftPanel->setMaximumWidth(320);
    leftPanel->setStyleSheet("background:#181825;");
    auto* leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(8, 8, 8, 8);
    leftLay->setSpacing(8);

    auto* grpAdj = new QGroupBox("Image Adjustments");
    auto* aLay = new QFormLayout(grpAdj);
    sliderBright = makeSlider(0, 100, 50);
    sliderContrast = makeSlider(0, 100, 50);
    sliderBlur = makeSlider(0, 15, 0);
    sliderRotate = makeSlider(0, 360, 0);
    addSliderRowI(aLay, "Brightness", sliderBright);
    addSliderRowI(aLay, "Contrast", sliderContrast);
    addSliderRowI(aLay, "Blur", sliderBlur);
    addSliderRowI(aLay, "Rotation", sliderRotate, "°");
    comboFlip = new QComboBox;
    comboFlip->addItems({ "None","Horizontal","Vertical","Both" });
    aLay->addRow(new QLabel("Flip:"), comboFlip);
    leftLay->addWidget(grpAdj);

    auto* grpBrush = new QGroupBox("Brush / Tool");
    auto* bLay = new QFormLayout(grpBrush);
    comboTool = new QComboBox;
    comboTool->addItems({ "View","Crop","Brush","Fill" });
    bLay->addRow(new QLabel("Mode:"), comboTool);
    sliderBrushSz = makeSlider(1, 50, 10);
    addSliderRowI(bLay, "Brush Size", sliderBrushSz);
    comboBrushColor = new QComboBox;
    comboBrushColor->addItems({ "Red","Green","Blue","Black","White" });
    bLay->addRow(new QLabel("Color:"), comboBrushColor);
    leftLay->addWidget(grpBrush);

    auto* btnApply = new QPushButton("Apply");
    leftLay->addWidget(btnApply);
    leftLay->addStretch();

    auto* btnBack = new QPushButton("Back");
    btnBack->setStyleSheet("QPushButton{background:#313244;color:#cdd6f4;}QPushButton:hover{background:#45475a;}");
    leftLay->addWidget(btnBack);

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

    connect(sliderBright, &QSlider::valueChanged, this, &InteractiveToolsWindow::applyInteractiveTool);
    connect(sliderContrast, &QSlider::valueChanged, this, &InteractiveToolsWindow::applyInteractiveTool);
    connect(sliderBlur, &QSlider::valueChanged, this, &InteractiveToolsWindow::applyInteractiveTool);
    connect(sliderRotate, &QSlider::valueChanged, this, &InteractiveToolsWindow::applyInteractiveTool);
    connect(comboFlip, &QComboBox::currentIndexChanged, this, &InteractiveToolsWindow::applyInteractiveTool);
    connect(btnApply, &QPushButton::clicked, this, &InteractiveToolsWindow::applyInteractiveTool);
    connect(btnBack, &QPushButton::clicked, this, &InteractiveToolsWindow::onBack);
}

void InteractiveToolsWindow::setupMenuBar()
{
    auto* mb = menuBar();
    auto* fm = mb->addMenu("&File");

    actOpen = new QAction("&Open…", this);
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &InteractiveToolsWindow::openImage);
    fm->addAction(actOpen);

    actSave = new QAction("&Save…", this);
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave, &QAction::triggered, this, &InteractiveToolsWindow::saveImage);
    fm->addAction(actSave);

    fm->addSeparator();
    auto* actQuit = new QAction("&Quit", this);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actQuit, &QAction::triggered, qApp, &QApplication::quit);
    fm->addAction(actQuit);

    auto* em = mb->addMenu("&Edit");
    actUndo = new QAction("&Undo", this);
    actUndo->setShortcut(QKeySequence::Undo);
    connect(actUndo, &QAction::triggered, this, &InteractiveToolsWindow::undoAction);
    em->addAction(actUndo);

    actRedo = new QAction("&Redo", this);
    actRedo->setShortcut(QKeySequence::Redo);
    connect(actRedo, &QAction::triggered, this, &InteractiveToolsWindow::redoAction);
    em->addAction(actRedo);

    em->addSeparator();
    actReset = new QAction("&Reset", this);
    connect(actReset, &QAction::triggered, this, &InteractiveToolsWindow::resetImage);
    em->addAction(actReset);

    actSave->setEnabled(false);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);
    actReset->setEnabled(false);
}

void InteractiveToolsWindow::setupToolBar()
{
    auto* tb = addToolBar("Toolbar");
    tb->setMovable(false);
    tb->addAction(actOpen); tb->addAction(actSave);
    tb->addSeparator();
    tb->addAction(actUndo); tb->addAction(actRedo);
    tb->addSeparator();
    tb->addAction(actReset);
}

void InteractiveToolsWindow::setupImageCanvas()
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
        "<p style='font-size:18px;margin:8px 0 4px;'>Ouvrez une image pour commencer</p>"
        "</div>"
    );
    imageLabel->setTextFormat(Qt::RichText);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
}

void InteractiveToolsWindow::setupStatusBar()
{
    statusLabel = new QLabel("No image loaded");
    statusBar()->addWidget(statusLabel, 1);
}

void InteractiveToolsWindow::showImage(const QImage& img)
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

void InteractiveToolsWindow::updateStatusBar()
{
    if (originalImage.isNull()) { statusLabel->setText("No image loaded"); return; }
    QString s = QString("%1 × %2 px").arg(originalImage.width()).arg(originalImage.height());
    if (!currentFilePath.isEmpty())
        s += "  ·  " + currentFilePath.section('\\', -1).section('/', -1);
    s += "  ·  Undo: " + QString::number(undoStack.size());
    statusLabel->setText(s);
}

void InteractiveToolsWindow::pushUndo()
{
    if (originalImage.isNull()) return;
    undoStack.push(currentImage.copy());
    if (undoStack.size() > 20) undoStack.removeFirst();
    redoStack.clear();
    actUndo->setEnabled(true);
    actRedo->setEnabled(false);
}

void InteractiveToolsWindow::openImage()
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
    applyInteractiveTool();
}

void InteractiveToolsWindow::saveImage()
{
    if (currentImage.isNull()) return;
    QString path = QFileDialog::getSaveFileName(this, "Save Image", "",
        "JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)");
    if (path.isEmpty()) return;
    if (currentImage.save(path)) statusLabel->setText("Saved: " + path);
    else QMessageBox::warning(this, "Error", "Unable to save image.");
}

void InteractiveToolsWindow::undoAction()
{
    if (undoStack.isEmpty()) return;
    redoStack.push(currentImage.copy());
    currentImage = undoStack.top();
    undoStack.pop();
    showImage(currentImage);
    actUndo->setEnabled(!undoStack.isEmpty());
    actRedo->setEnabled(true);
    updateStatusBar();
}

void InteractiveToolsWindow::redoAction()
{
    if (redoStack.isEmpty()) return;
    undoStack.push(currentImage.copy());
    currentImage = redoStack.top();
    redoStack.pop();
    showImage(currentImage);
    actRedo->setEnabled(!redoStack.isEmpty());
    actUndo->setEnabled(true);
    updateStatusBar();
}

void InteractiveToolsWindow::resetImage()
{
    if (originalImage.isNull()) return;
    pushUndo();
    currentImage = originalImage.copy();
    sliderBright->setValue(50);
    sliderContrast->setValue(50);
    sliderBlur->setValue(0);
    sliderRotate->setValue(0);
    comboFlip->setCurrentIndex(0);
    showImage(currentImage);
    updateStatusBar();
}

void InteractiveToolsWindow::onBack()
{
    if (m_startInterface) m_startInterface->show();
    close();
}

void InteractiveToolsWindow::applyInteractiveTool()
{
    if (originalImage.isNull()) return;

    QImage img = originalImage.convertToFormat(QImage::Format_ARGB32);
    QImage result = img.copy();

    double alpha = 0.5 + (sliderContrast->value() / 50.0);
    int    beta = (sliderBright->value() - 50) * 2;
    if (alpha != 1.0 || beta != 0) {
        for (int y = 0; y < img.height(); y++) {
            const QRgb* s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb* d = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++)
                d[x] = qRgb(qBound(0, (int)(qRed(s[x]) * alpha + beta), 255),
                    qBound(0, (int)(qGreen(s[x]) * alpha + beta), 255),
                    qBound(0, (int)(qBlue(s[x]) * alpha + beta), 255));
        }
    }

    int br = sliderBlur->value();
    if (br > 0) {
        int factor = 1 << br;
        factor = qMin(factor, qMin(img.width(), img.height()) / 4);
        factor = qMax(2, factor);
        QImage small = result.scaled(result.width() / factor, result.height() / factor,
            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        result = small.scaled(result.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    int angle = sliderRotate->value();
    if (angle != 0) {
        QTransform t; t.rotate(angle);
        result = result.transformed(t, Qt::SmoothTransformation);
    }

    switch (comboFlip->currentIndex()) {
    case 1: result = result.mirrored(Qt::Horizontal); break;
    case 2: result = result.mirrored(Qt::Vertical);   break;
    case 3: result = result.mirrored(Qt::Horizontal).mirrored(Qt::Vertical); break;
    }

    currentImage = result.convertToFormat(QImage::Format_RGB888);
    showImage(currentImage);
}