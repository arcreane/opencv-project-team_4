#include "mainwindow.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QAction>
#include <QSizePolicy>
#include <QPainter>
#include <QResizeEvent>
#include <cstdlib>
#include <ctime>

// ─── Helpers ─────────────────────────────────────────────────────────────────

QLabel *MainWindow::makeLabel(const QString &text)
{
    auto *l = new QLabel(text);
    l->setStyleSheet("color:#a6adc8; font-size:12px;");
    return l;
}

QSlider *MainWindow::makeSlider(int min, int max, int val, Qt::Orientation orient)
{
    auto *s = new QSlider(orient);
    s->setRange(min, max);
    s->setValue(val);
    s->setStyleSheet(
        "QSlider::groove:horizontal{background:#45475a;height:4px;border-radius:2px;}"
        "QSlider::handle:horizontal{background:#89b4fa;width:14px;height:14px;margin:-5px 0;border-radius:7px;}"
        "QSlider::sub-page:horizontal{background:#89b4fa;height:4px;border-radius:2px;}"
    );
    return s;
}

// ─── Constructor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    std::srand(42);
    setWindowTitle("Éditeur d'Images — ISEP 2026");
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
        "QTabWidget::pane{border:1px solid #45475a;border-radius:4px;}"
        "QTabBar::tab{background:#313244;color:#a6adc8;padding:8px 14px;border-top-left-radius:6px;border-top-right-radius:6px;margin-right:2px;}"
        "QTabBar::tab:selected{background:#45475a;color:#cdd6f4;font-weight:bold;}"
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

MainWindow::~MainWindow() {}

// ─── Auto-preview helper ──────────────────────────────────────────────────────

// Called by every slider/combo — applies the current tab's effect live
void MainWindow::livePreview()
{
    if (originalImage.isNull()) return;
    int tab = tabWidget->currentIndex();
    if      (tab == 0) applyThreshold();
    else if (tab == 1) applyEffect();
    else if (tab == 2) applyInteractiveTool();
}

// ─── Setup ───────────────────────────────────────────────────────────────────

void MainWindow::setupUI()
{
    setupMenuBar();
    setupToolBar();

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet("QSplitter::handle{background:#45475a;}");

    // Left panel
    auto *leftPanel = new QWidget;
    leftPanel->setMinimumWidth(270);
    leftPanel->setMaximumWidth(320);
    leftPanel->setStyleSheet("background:#181825;");
    auto *leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(8,8,8,8);

    tabWidget = new QTabWidget;
    auto *tabThresh = new QWidget;
    auto *tabFx     = new QWidget;
    auto *tabTools  = new QWidget;
    setupThresholdingTab(tabThresh);
    setupCreativeEffectsTab(tabFx);
    setupInteractiveToolsTab(tabTools);
    tabWidget->addTab(tabThresh, "Seuillage");
    tabWidget->addTab(tabFx,    "Effets");
    tabWidget->addTab(tabTools, "Outils");
    // Switch tab → re-apply on new tab
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int){ livePreview(); });

    leftLay->addWidget(tabWidget);

    // Right panel
    auto *rightPanel = new QWidget;
    setupImageCanvas();
    auto *rightLay = new QVBoxLayout(rightPanel);
    rightLay->setContentsMargins(0,0,0,0);
    rightLay->addWidget(scrollArea);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0,0);
    splitter->setStretchFactor(1,1);
    mainLay->addWidget(splitter);

    setupStatusBar();
}

void MainWindow::setupMenuBar()
{
    auto *mb = menuBar();
    auto *fm = mb->addMenu("&Fichier");

    actOpen = new QAction("&Ouvrir…", this);
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &MainWindow::openImage);
    fm->addAction(actOpen);

    actSave = new QAction("&Enregistrer…", this);
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave, &QAction::triggered, this, &MainWindow::saveImage);
    fm->addAction(actSave);

    fm->addSeparator();
    auto *actQuit = new QAction("&Quitter", this);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actQuit, &QAction::triggered, qApp, &QApplication::quit);
    fm->addAction(actQuit);

    auto *em = mb->addMenu("&Édition");

    actUndo = new QAction("&Annuler", this);
    actUndo->setShortcut(QKeySequence::Undo);
    connect(actUndo, &QAction::triggered, this, &MainWindow::undoAction);
    em->addAction(actUndo);

    actRedo = new QAction("&Rétablir", this);
    actRedo->setShortcut(QKeySequence::Redo);
    connect(actRedo, &QAction::triggered, this, &MainWindow::redoAction);
    em->addAction(actRedo);

    em->addSeparator();
    actReset = new QAction("&Réinitialiser", this);
    connect(actReset, &QAction::triggered, this, &MainWindow::resetImage);
    em->addAction(actReset);

    actSave->setEnabled(false);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);
    actReset->setEnabled(false);
}

void MainWindow::setupToolBar()
{
    auto *tb = addToolBar("Barre d'outils");
    tb->setMovable(false);
    tb->addAction(actOpen);
    tb->addAction(actSave);
    tb->addSeparator();
    tb->addAction(actUndo);
    tb->addAction(actRedo);
    tb->addSeparator();
    tb->addAction(actReset);
}

// ─── Value label helper ───────────────────────────────────────────────────────

// Creates a slider + live value label row in a QFormLayout
static QLabel *addSliderRow(QFormLayout *form, const QString &name,
                            QSlider *slider, const QString &suffix = "")
{
    auto *lbl = new QLabel(QString::number(slider->value()) + suffix);
    lbl->setObjectName("valueLabel");
    lbl->setMinimumWidth(40);
    auto *row = new QHBoxLayout;
    row->addWidget(slider);
    row->addWidget(lbl);
    form->addRow(new QLabel(name + ":"), row);
    QObject::connect(slider, &QSlider::valueChanged, lbl,
        [lbl, suffix](int v){ lbl->setText(QString::number(v) + suffix); });
    return lbl;
}

// ─── Tab 1: Thresholding ──────────────────────────────────────────────────────

void MainWindow::setupThresholdingTab(QWidget *tab)
{
    auto *lay = new QVBoxLayout(tab);
    lay->setContentsMargins(8,8,8,8);
    lay->setSpacing(8);

    auto *grpMode = new QGroupBox("Mode de seuillage");
    auto *mLay = new QVBoxLayout(grpMode);
    comboThreshMode = new QComboBox;
    comboThreshMode->addItems({"Binary","Binary Inv","Trunc","ToZero","ToZero Inv","Otsu","Adaptive Gauss","Adaptive Mean"});
    mLay->addWidget(comboThreshMode);
    lay->addWidget(grpMode);

    auto *grpParams = new QGroupBox("Paramètres");
    auto *pLay = new QFormLayout(grpParams);
    sliderThresh = makeSlider(0, 255, 127);
    addSliderRow(pLay, "Seuil", sliderThresh);
    sliderBlock  = makeSlider(3, 51, 11);
    addSliderRow(pLay, "Block size", sliderBlock);
    sliderConstC = makeSlider(0, 20, 2);
    addSliderRow(pLay, "Constante C", sliderConstC);
    lay->addWidget(grpParams);

    checkInvert = new QCheckBox("Inverser le résultat");
    lay->addWidget(checkInvert);

    auto *btnApply = new QPushButton("Appliquer");
    lay->addWidget(btnApply);
    lay->addStretch();

    // ── Live preview connections ──
    connect(comboThreshMode, &QComboBox::currentIndexChanged, this, [this](int idx){
        onThreshModeChanged(idx); livePreview();
    });
    connect(sliderThresh,  &QSlider::valueChanged,     this, [this]{ livePreview(); });
    connect(sliderBlock,   &QSlider::valueChanged,     this, [this]{ livePreview(); });
    connect(sliderConstC,  &QSlider::valueChanged,     this, [this]{ livePreview(); });
    connect(checkInvert,   &QCheckBox::stateChanged,   this, [this]{ livePreview(); });
    connect(btnApply,      &QPushButton::clicked,      this, &MainWindow::applyThreshold);

    onThreshModeChanged(0);
}

// ─── Tab 2: Creative Effects ──────────────────────────────────────────────────

void MainWindow::setupCreativeEffectsTab(QWidget *tab)
{
    auto *lay = new QVBoxLayout(tab);
    lay->setContentsMargins(8,8,8,8);
    lay->setSpacing(8);

    auto *grpFx = new QGroupBox("Effet créatif");
    auto *fLay  = new QVBoxLayout(grpFx);
    comboEffect = new QComboBox;
    comboEffect->addItems({"Cartoon","Pencil Sketch","Vignette","Film Grain",
                           "Color Tint","Sépia","Emboss","Pixelate","Négatif","Warm/Cool"});
    fLay->addWidget(comboEffect);
    lay->addWidget(grpFx);

    auto *grpParams = new QGroupBox("Paramètres");
    auto *pLay = new QFormLayout(grpParams);

    comboTint = new QComboBox;
    comboTint->addItems({"Rouge","Vert","Bleu","Jaune","Violet"});
    pLay->addRow(new QLabel("Teinte:"), comboTint);

    sliderGrain    = makeSlider(1, 80, 30);
    sliderPixel    = makeSlider(2, 40, 10);
    sliderWarmCool = makeSlider(0, 100, 50);
    addSliderRow(pLay, "Grain",      sliderGrain);
    addSliderRow(pLay, "Taille pixel", sliderPixel);
    addSliderRow(pLay, "Warm/Cool",  sliderWarmCool);
    lay->addWidget(grpParams);

    auto *btnApply = new QPushButton("Appliquer");
    lay->addWidget(btnApply);
    lay->addStretch();

    // ── Live preview ──
    connect(comboEffect,   &QComboBox::currentIndexChanged, this, [this]{ livePreview(); });
    connect(comboTint,     &QComboBox::currentIndexChanged, this, [this]{ livePreview(); });
    connect(sliderGrain,   &QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(sliderPixel,   &QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(sliderWarmCool,&QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(btnApply,      &QPushButton::clicked,           this, &MainWindow::applyEffect);
}

// ─── Tab 3: Interactive Tools ─────────────────────────────────────────────────

void MainWindow::setupInteractiveToolsTab(QWidget *tab)
{
    auto *lay = new QVBoxLayout(tab);
    lay->setContentsMargins(8,8,8,8);
    lay->setSpacing(8);

    auto *grpAdj = new QGroupBox("Ajustements image");
    auto *aLay   = new QFormLayout(grpAdj);

    sliderBright   = makeSlider(0, 100, 50);
    sliderContrast = makeSlider(0, 100, 50);
    sliderBlur     = makeSlider(0, 15,  0);
    sliderRotate   = makeSlider(0, 360, 0);

    addSliderRow(aLay, "Luminosité",  sliderBright);
    addSliderRow(aLay, "Contraste",   sliderContrast);
    addSliderRow(aLay, "Flou",        sliderBlur);
    addSliderRow(aLay, "Rotation",    sliderRotate, "°");

    comboFlip = new QComboBox;
    comboFlip->addItems({"Aucun","Horizontal","Vertical","Les deux"});
    aLay->addRow(new QLabel("Miroir:"), comboFlip);
    lay->addWidget(grpAdj);

    auto *grpBrush = new QGroupBox("Pinceau / Outil");
    auto *bLay     = new QFormLayout(grpBrush);
    comboTool = new QComboBox;
    comboTool->addItems({"Vue","Recadrage","Pinceau","Remplissage"});
    bLay->addRow(new QLabel("Mode:"), comboTool);
    sliderBrushSz = makeSlider(1, 50, 10);
    addSliderRow(bLay, "Taille pinceau", sliderBrushSz);
    comboBrushColor = new QComboBox;
    comboBrushColor->addItems({"Rouge","Vert","Bleu","Noir","Blanc"});
    bLay->addRow(new QLabel("Couleur:"), comboBrushColor);
    lay->addWidget(grpBrush);

    auto *btnApply = new QPushButton("Appliquer");
    lay->addWidget(btnApply);
    lay->addStretch();

    // ── Live preview ──
    connect(sliderBright,   &QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(sliderContrast, &QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(sliderBlur,     &QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(sliderRotate,   &QSlider::valueChanged,          this, [this]{ livePreview(); });
    connect(comboFlip,      &QComboBox::currentIndexChanged, this, [this]{ livePreview(); });
    connect(btnApply,       &QPushButton::clicked,           this, &MainWindow::applyInteractiveTool);
}

// ─── Image Canvas ─────────────────────────────────────────────────────────────

void MainWindow::setupImageCanvas()
{
    scrollArea = new QScrollArea;
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setStyleSheet("background:#11111b;border:none;");

    imageLabel = new QLabel;
    imageLabel->setObjectName("imageLabel");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    imageLabel->setText(
        "<div style='color:#585b70;text-align:center;'>"
        "<p style='font-size:48px;margin:0;'>🖼</p>"
        "<p style='font-size:18px;margin:8px 0 4px;'>Ouvrez une image pour commencer</p>"
        "<p style='font-size:13px;color:#45475a;'>Fichier → Ouvrir &nbsp;(Ctrl+O)</p>"
        "</div>"
    );
    imageLabel->setTextFormat(Qt::RichText);

    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
}

void MainWindow::setupStatusBar()
{
    statusLabel = new QLabel("Aucune image chargée");
    statusBar()->addWidget(statusLabel, 1);
}

// ─── Display ──────────────────────────────────────────────────────────────────

void MainWindow::showImage(const QImage &img)
{
    imageLabel->setTextFormat(Qt::PlainText);
    imageLabel->setText("");

    QSize available = scrollArea->viewport()->size();
    QPixmap pix = QPixmap::fromImage(img);

    if (pix.width() > available.width() || pix.height() > available.height())
        pix = pix.scaled(available, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    imageLabel->setPixmap(pix);
    imageLabel->resize(pix.size());
}

void MainWindow::updateStatusBar()
{
    if (originalImage.isNull()) { statusLabel->setText("Aucune image"); return; }
    QString s = QString("%1 × %2 px").arg(originalImage.width()).arg(originalImage.height());
    if (!currentFilePath.isEmpty())
        s += "  ·  " + currentFilePath.section('\\', -1).section('/', -1);
    s += "  ·  Undo: " + QString::number(undoStack.size());
    statusLabel->setText(s);
}

void MainWindow::pushUndo()
{
    if (originalImage.isNull()) return;
    undoStack.push(currentImage.copy());
    if (undoStack.size() > 20) undoStack.removeFirst();
    redoStack.clear();
    actUndo->setEnabled(true);
    actRedo->setEnabled(false);
    updateStatusBar();
}

// ─── File operations ──────────────────────────────────────────────────────────

void MainWindow::openImage()
{
    QString path = QFileDialog::getOpenFileName(this, "Ouvrir une image", "",
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff);;Tous les fichiers (*)");
    if (path.isEmpty()) return;

    QImage img(path);
    if (img.isNull()) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger l'image:\n" + path);
        return;
    }

    currentFilePath = path;
    img = img.convertToFormat(QImage::Format_RGB888);
    originalImage = img.copy();
    currentImage  = img.copy();
    undoStack.clear();
    redoStack.clear();
    actSave->setEnabled(true);
    actReset->setEnabled(true);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);

    showImage(currentImage);
    updateStatusBar();
    // Auto-apply current tab's settings on new image
    livePreview();
}

void MainWindow::saveImage()
{
    if (currentImage.isNull()) return;
    QString path = QFileDialog::getSaveFileName(this, "Enregistrer", "",
        "JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)");
    if (path.isEmpty()) return;
    if (currentImage.save(path))
        statusLabel->setText("Sauvegardé : " + path);
    else
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder.");
}

// ─── Undo / Redo / Reset ──────────────────────────────────────────────────────

void MainWindow::undoAction()
{
    if (undoStack.isEmpty()) return;
    redoStack.push(currentImage.copy());
    currentImage = undoStack.pop();
    showImage(currentImage);
    actUndo->setEnabled(!undoStack.isEmpty());
    actRedo->setEnabled(true);
    updateStatusBar();
}

void MainWindow::redoAction()
{
    if (redoStack.isEmpty()) return;
    undoStack.push(currentImage.copy());
    currentImage = redoStack.pop();
    showImage(currentImage);
    actRedo->setEnabled(!redoStack.isEmpty());
    actUndo->setEnabled(true);
    updateStatusBar();
}

void MainWindow::resetImage()
{
    if (originalImage.isNull()) return;
    pushUndo();
    currentImage = originalImage.copy();
    showImage(currentImage);
    updateStatusBar();
    // Reset all sliders to neutral
    sliderBright->setValue(50);
    sliderContrast->setValue(50);
    sliderBlur->setValue(0);
    sliderRotate->setValue(0);
    comboFlip->setCurrentIndex(0);
}

// ─── Thresholding ─────────────────────────────────────────────────────────────

void MainWindow::onThreshModeChanged(int idx)
{
    bool adaptive = (idx == 6 || idx == 7);
    sliderThresh->setEnabled(idx < 5);
    sliderBlock->setEnabled(adaptive);
    sliderConstC->setEnabled(adaptive);
}

void MainWindow::applyThreshold()
{
    if (originalImage.isNull()) return;

    QImage gray = originalImage.convertToFormat(QImage::Format_Grayscale8);
    int mode   = comboThreshMode->currentIndex();
    int thresh = sliderThresh->value();
    bool inv   = checkInvert->isChecked();

    QImage result(gray.size(), QImage::Format_Grayscale8);

    if (mode == 0 || mode == 1) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar *s = gray.constScanLine(y);
            uchar *d       = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++)
                d[x] = ((s[x] > thresh) ^ (mode == 1)) ? 255 : 0;
        }
    } else if (mode == 2) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar *s = gray.constScanLine(y); uchar *d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] > thresh ? (uchar)thresh : s[x];
        }
    } else if (mode == 3) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar *s = gray.constScanLine(y); uchar *d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] > thresh ? s[x] : 0;
        }
    } else if (mode == 4) {
        for (int y = 0; y < gray.height(); y++) {
            const uchar *s = gray.constScanLine(y); uchar *d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] <= thresh ? s[x] : 0;
        }
    } else if (mode == 5) { // Otsu
        long long hist[256] = {};
        for (int y = 0; y < gray.height(); y++) {
            const uchar *s = gray.constScanLine(y);
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
            double mB = sumB/wB, mF = (sum-sumB)/wF;
            double var = wB*wF*(mB-mF)*(mB-mF);
            if (var > maxVar) { maxVar = var; otsuT = i; }
        }
        for (int y = 0; y < gray.height(); y++) {
            const uchar *s = gray.constScanLine(y); uchar *d = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) d[x] = s[x] > otsuT ? 255 : 0;
        }
    } else { // Adaptive (6, 7)
        int bs = sliderBlock->value() | 1; // ensure odd
        bs = qMax(3, bs);
        int C = sliderConstC->value(), half = bs/2;
        for (int y = 0; y < gray.height(); y++) {
            const uchar *src = gray.constScanLine(y);
            uchar *dst = result.scanLine(y);
            for (int x = 0; x < gray.width(); x++) {
                long long s2 = 0, cnt = 0;
                for (int dy = -half; dy <= half; dy++) {
                    const uchar *row = gray.constScanLine(qBound(0, y+dy, gray.height()-1));
                    for (int dx = -half; dx <= half; dx++) {
                        s2 += row[qBound(0, x+dx, gray.width()-1)]; cnt++;
                    }
                }
                dst[x] = src[x] > (s2/cnt - C) ? 255 : 0;
            }
        }
    }

    if (inv) result.invertPixels();
    currentImage = result.convertToFormat(QImage::Format_RGB888);
    showImage(currentImage);
}

// ─── Creative Effects ─────────────────────────────────────────────────────────

void MainWindow::onEffectModeChanged(int) {}

void MainWindow::applyEffect()
{
    if (originalImage.isNull()) return;

    QImage img = originalImage.convertToFormat(QImage::Format_RGB888);
    QImage result = img.copy();
    int mode = comboEffect->currentIndex();

    switch (mode) {
    case 3: { // Film Grain
        int amount = sliderGrain->value();
        for (int y = 0; y < img.height(); y++) {
            const QRgb *s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb *d       = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++) {
                int n = (std::rand() % (amount*2+1)) - amount;
                d[x] = qRgb(qBound(0,qRed(s[x])+n,255),
                            qBound(0,qGreen(s[x])+n,255),
                            qBound(0,qBlue(s[x])+n,255));
            }
        }
        break;
    }
    case 4: { // Color Tint
        QColor tints[]={{180,0,0},{0,180,0},{0,0,180},{160,160,0},{140,0,140}};
        QColor tc = tints[comboTint->currentIndex()];
        QImage overlay(img.size(), QImage::Format_ARGB32);
        overlay.fill(QColor(tc.red(),tc.green(),tc.blue(),70));
        QPainter p(&result); p.drawImage(0,0,overlay);
        break;
    }
    case 5: { // Sepia
        for (int y = 0; y < img.height(); y++) {
            const QRgb *s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb *d       = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++) {
                int r=qRed(s[x]),g=qGreen(s[x]),b=qBlue(s[x]);
                d[x]=qRgb(qBound(0,(int)(r*.393+g*.769+b*.189),255),
                          qBound(0,(int)(r*.349+g*.686+b*.168),255),
                          qBound(0,(int)(r*.272+g*.534+b*.131),255));
            }
        }
        break;
    }
    case 7: { // Pixelate
        int ps = qMax(2, sliderPixel->value());
        for (int y = 0; y < img.height(); y += ps)
            for (int x = 0; x < img.width(); x += ps) {
                QRgb c = img.pixel(x, y);
                for (int dy=0; dy<ps && y+dy<img.height(); dy++)
                    for (int dx=0; dx<ps && x+dx<img.width(); dx++)
                        result.setPixel(x+dx, y+dy, c);
            }
        break;
    }
    case 8: { // Negative
        result.invertPixels();
        break;
    }
    case 9: { // Warm/Cool
        int shift = sliderWarmCool->value() - 50;
        for (int y = 0; y < img.height(); y++) {
            const QRgb *s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb *d       = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++)
                d[x] = qRgb(qBound(0,qRed(s[x])+shift,255),
                            qGreen(s[x]),
                            qBound(0,qBlue(s[x])-shift,255));
        }
        break;
    }
    case 2: { // Vignette
        result = result.convertToFormat(QImage::Format_ARGB32);
        QPainter p(&result);
        QRadialGradient g(img.width()/2.0, img.height()/2.0,
                          qMax(img.width(), img.height()) * 0.6);
        g.setColorAt(0.0, QColor(0,0,0,0));
        g.setColorAt(1.0, QColor(0,0,0,180));
        p.fillRect(result.rect(), g);
        result = result.convertToFormat(QImage::Format_RGB888);
        break;
    }
    default:
        // Cartoon, Sketch, Emboss require OpenCV — show original with message
        statusLabel->setText(QString("'%1' requiert OpenCV — non disponible en mode Qt pur").arg(comboEffect->currentText()));
        currentImage = originalImage.copy();
        showImage(currentImage);
        return;
    }

    currentImage = result;
    showImage(currentImage);
}

// ─── Interactive Tools ────────────────────────────────────────────────────────

void MainWindow::applyInteractiveTool()
{
    if (originalImage.isNull()) return;

    QImage img = originalImage.convertToFormat(QImage::Format_RGB888);
    QImage result = img.copy();

    // Brightness / Contrast
    double alpha = 0.5 + (sliderContrast->value() / 50.0);
    int    beta  = (sliderBright->value() - 50) * 2;
    if (alpha != 1.0 || beta != 0) {
        for (int y = 0; y < img.height(); y++) {
            const QRgb *s = reinterpret_cast<const QRgb*>(img.constScanLine(y));
            QRgb *d       = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < img.width(); x++)
                d[x] = qRgb(qBound(0,(int)(qRed(s[x])*alpha+beta),255),
                            qBound(0,(int)(qGreen(s[x])*alpha+beta),255),
                            qBound(0,(int)(qBlue(s[x])*alpha+beta),255));
        }
    }

    // Blur — use Qt's smooth transform trick (fast)
    int br = sliderBlur->value();
    if (br > 0) {
        int factor = 1 << br; // scale down then up = blur
        factor = qMin(factor, qMin(img.width(), img.height()) / 4);
        factor = qMax(2, factor);
        QImage small = result.scaled(
            result.width() / factor, result.height() / factor,
            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        result = small.scaled(result.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Rotation
    int angle = sliderRotate->value();
    if (angle != 0) {
        QTransform t; t.rotate(angle);
        result = result.transformed(t, Qt::SmoothTransformation);
    }

    // Flip
    switch (comboFlip->currentIndex()) {
    case 1: result = result.flipped(Qt::Horizontal); break;
    case 2: result = result.flipped(Qt::Vertical);   break;
    case 3: result = result.flipped(Qt::Horizontal).flipped(Qt::Vertical); break;
    }

    currentImage = result.convertToFormat(QImage::Format_RGB888);
    showImage(currentImage);
}
