#include "CreativeEffectsWindow.h"
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
#include <QPainter>
#include <QRadialGradient>
#include <cstdlib>
#include <ctime>

static QLabel* addSliderRowC(QFormLayout* form, const QString& name, QSlider* slider, const QString& suffix = "")
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

CreativeEffectsWindow::CreativeEffectsWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_startInterface(qobject_cast<StartInterface*>(parent))
{
    std::srand(42);
    setWindowTitle("Creative Effects");
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

CreativeEffectsWindow::~CreativeEffectsWindow() {}

void CreativeEffectsWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (!currentImage.isNull()) showImage(currentImage);
}

QSlider* CreativeEffectsWindow::makeSlider(int min, int max, int val, Qt::Orientation orient)
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

void CreativeEffectsWindow::setupUI()
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

    auto* grpFx = new QGroupBox("Creative Effect");
    auto* fLay = new QVBoxLayout(grpFx);
    comboEffect = new QComboBox;
    comboEffect->addItems({ "Cartoon","Pencil Sketch","Vignette","Film Grain",
                           "Color Tint","Sepia","Emboss","Pixelate","Negative","Warm/Cool" });
    fLay->addWidget(comboEffect);
    leftLay->addWidget(grpFx);

    auto* grpParams = new QGroupBox("Parameters");
    auto* pLay = new QFormLayout(grpParams);

    comboTint = new QComboBox;
    comboTint->addItems({ "Red","Green","Blue","Yellow","Purple" });
    pLay->addRow(new QLabel("Tint:"), comboTint);

    sliderGrain = makeSlider(1, 80, 30);
    sliderPixel = makeSlider(2, 40, 10);
    sliderWarmCool = makeSlider(0, 100, 50);
    addSliderRowC(pLay, "Grain", sliderGrain);
    addSliderRowC(pLay, "Pixel Size", sliderPixel);
    addSliderRowC(pLay, "Warm/Cool", sliderWarmCool);
    leftLay->addWidget(grpParams);

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

    connect(comboEffect, &QComboBox::currentIndexChanged, this, &CreativeEffectsWindow::applyEffect);
    connect(comboTint, &QComboBox::currentIndexChanged, this, &CreativeEffectsWindow::applyEffect);
    connect(sliderGrain, &QSlider::valueChanged, this, &CreativeEffectsWindow::applyEffect);
    connect(sliderPixel, &QSlider::valueChanged, this, &CreativeEffectsWindow::applyEffect);
    connect(sliderWarmCool, &QSlider::valueChanged, this, &CreativeEffectsWindow::applyEffect);
    connect(btnApply, &QPushButton::clicked, this, &CreativeEffectsWindow::applyEffect);
    connect(btnBack, &QPushButton::clicked, this, &CreativeEffectsWindow::onBack);
}

void CreativeEffectsWindow::setupMenuBar()
{
    auto* mb = menuBar();
    auto* fm = mb->addMenu("&File");

    actOpen = new QAction("&Open…", this);
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &CreativeEffectsWindow::openImage);
    fm->addAction(actOpen);

    actSave = new QAction("&Save…", this);
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave, &QAction::triggered, this, &CreativeEffectsWindow::saveImage);
    fm->addAction(actSave);

    fm->addSeparator();
    auto* actQuit = new QAction("&Quit", this);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actQuit, &QAction::triggered, qApp, &QApplication::quit);
    fm->addAction(actQuit);

    auto* em = mb->addMenu("&Edit");
    actUndo = new QAction("&Undo", this);
    actUndo->setShortcut(QKeySequence::Undo);
    connect(actUndo, &QAction::triggered, this, &CreativeEffectsWindow::undoAction);
    em->addAction(actUndo);

    actRedo = new QAction("&Redo", this);
    actRedo->setShortcut(QKeySequence::Redo);
    connect(actRedo, &QAction::triggered, this, &CreativeEffectsWindow::redoAction);
    em->addAction(actRedo);

    em->addSeparator();
    actReset = new QAction("&Reset", this);
    connect(actReset, &QAction::triggered, this, &CreativeEffectsWindow::resetImage);
    em->addAction(actReset);

    actSave->setEnabled(false);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);
    actReset->setEnabled(false);
}

void CreativeEffectsWindow::setupToolBar()
{
    auto* tb = addToolBar("Toolbar");
    tb->setMovable(false);
    tb->addAction(actOpen); tb->addAction(actSave);
    tb->addSeparator();
    tb->addAction(actUndo); tb->addAction(actRedo);
    tb->addSeparator();
    tb->addAction(actReset);
}

void CreativeEffectsWindow::setupImageCanvas()
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

void CreativeEffectsWindow::setupStatusBar()
{
    statusLabel = new QLabel("No image loaded");
    statusBar()->addWidget(statusLabel, 1);
}

void CreativeEffectsWindow::showImage(const QImage& img)
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

void CreativeEffectsWindow::updateStatusBar()
{
    if (originalImage.isNull()) { statusLabel->setText("No image loaded"); return; }
    QString s = QString("%1 × %2 px").arg(originalImage.width()).arg(originalImage.height());
    if (!currentFilePath.isEmpty())
        s += "  ·  " + currentFilePath.section('\\', -1).section('/', -1);
    statusLabel->setText(s);
}

void CreativeEffectsWindow::pushUndo()
{
    if (originalImage.isNull()) return;
    undoStack.push(currentImage.copy());
    if (undoStack.size() > 20) undoStack.removeFirst();
    redoStack.clear();
    actUndo->setEnabled(true);
    actRedo->setEnabled(false);
}

void CreativeEffectsWindow::openImage()
{
    QString path = QFileDialog::getOpenFileName(this, "Load an image", "",
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff);;All Files (*)");
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
    applyEffect();
}

void CreativeEffectsWindow::saveImage()
{
    if (currentImage.isNull()) return;
    QString path = QFileDialog::getSaveFileName(this, "Save", "",
        "JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)");
    if (path.isEmpty()) return;
    if (currentImage.save(path)) statusLabel->setText("Saved : " + path);
    else QMessageBox::warning(this, "Error", "Unable to save.");
}

void CreativeEffectsWindow::undoAction()
{
    if (undoStack.isEmpty()) return;
    redoStack.push(currentImage.copy());
    currentImage = undoStack.pop();
    showImage(currentImage);
    actUndo->setEnabled(!undoStack.isEmpty());
    actRedo->setEnabled(true);
}

void CreativeEffectsWindow::redoAction()
{
    if (redoStack.isEmpty()) return;
    undoStack.push(currentImage.copy());
    currentImage = redoStack.pop();
    showImage(currentImage);
    actRedo->setEnabled(!redoStack.isEmpty());
    actUndo->setEnabled(true);
}

void CreativeEffectsWindow::resetImage()
{
    if (originalImage.isNull()) return;
    pushUndo();
    currentImage = originalImage.copy();
    showImage(currentImage);
}

void CreativeEffectsWindow::onBack()
{
    if (m_startInterface) m_startInterface->show();
    close();
}

void CreativeEffectsWindow::applyEffect()
{
    if (originalImage.isNull()) return;

    QImage img = originalImage.convertToFormat(QImage::Format_RGB888);
    QImage result = img.copy();
    int mode = comboEffect->currentIndex();

    switch (mode) {
    case 3: {
        int amount = sliderGrain->value();
        for (int y = 0; y < img.height(); y++) {
            const QRgb* s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb* d = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++) {
                int n = (std::rand() % (amount * 2 + 1)) - amount;
                d[x] = qRgb(qBound(0, qRed(s[x]) + n, 255),
                    qBound(0, qGreen(s[x]) + n, 255),
                    qBound(0, qBlue(s[x]) + n, 255));
            }
        }
        break;
    }
    case 4: {
        QColor tints[] = { {180,0,0},{0,180,0},{0,0,180},{160,160,0},{140,0,140} };
        QColor tc = tints[comboTint->currentIndex()];
        QImage overlay(img.size(), QImage::Format_ARGB32);
        overlay.fill(QColor(tc.red(), tc.green(), tc.blue(), 70));
        QPainter p(&result); p.drawImage(0, 0, overlay);
        break;
    }
    case 5: {
        for (int y = 0; y < img.height(); y++) {
            const QRgb* s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb* d = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++) {
                int r = qRed(s[x]), g = qGreen(s[x]), b = qBlue(s[x]);
                d[x] = qRgb(qBound(0, (int)(r * .393 + g * .769 + b * .189), 255),
                    qBound(0, (int)(r * .349 + g * .686 + b * .168), 255),
                    qBound(0, (int)(r * .272 + g * .534 + b * .131), 255));
            }
        }
        break;
    }
    case 7: {
        int ps = qMax(2, sliderPixel->value());
        for (int y = 0; y < img.height(); y += ps)
            for (int x = 0; x < img.width(); x += ps) {
                QRgb c = img.pixel(x, y);
                for (int dy = 0; dy < ps && y + dy < img.height(); dy++)
                    for (int dx = 0; dx < ps && x + dx < img.width(); dx++)
                        result.setPixel(x + dx, y + dy, c);
            }
        break;
    }
    case 8: {
        result.invertPixels();
        break;
    }
    case 9: {
        int shift = sliderWarmCool->value() - 50;
        for (int y = 0; y < img.height(); y++) {
            const QRgb* s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb* d = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++)
                d[x] = qRgb(qBound(0, qRed(s[x]) + shift, 255),
                    qGreen(s[x]),
                    qBound(0, qBlue(s[x]) - shift, 255));
        }
        break;
    }
    case 2: {
        result = result.convertToFormat(QImage::Format_ARGB32);
        {
            QPainter p(&result);
            QRadialGradient g(img.width() / 2.0, img.height() / 2.0,
                qMax(img.width(), img.height()) * 0.6);
            g.setColorAt(0.0, QColor(0, 0, 0, 0));
            g.setColorAt(1.0, QColor(0, 0, 0, 180));
            p.fillRect(result.rect(), g);
        } 
        result = result.convertToFormat(QImage::Format_RGB888);
        break;
    }
    default:
        statusLabel->setText(QString("'%1' requiert OpenCV").arg(comboEffect->currentText()));
        currentImage = originalImage.copy();
        showImage(currentImage);
        return;
    }

    currentImage = result;
    showImage(currentImage);
}