#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QScrollArea>
#include <QCheckBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QAction>
#include <QPixmap>
#include <QImage>
#include <QString>
#include <QStack>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void openImage();
    void saveImage();
    void undoAction();
    void redoAction();
    void resetImage();
    void livePreview();

    // Thresholding
    void applyThreshold();
    void onThreshModeChanged(int idx);

    // Creative Effects
    void applyEffect();
    void onEffectModeChanged(int idx);

    // Interactive Tools
    void applyInteractiveTool();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupLeftPanel();
    void setupThresholdingTab(QWidget *tab);
    void setupCreativeEffectsTab(QWidget *tab);
    void setupInteractiveToolsTab(QWidget *tab);
    void setupImageCanvas();
    void setupStatusBar();

    void showImage(const QImage &img);
    void pushUndo();
    void updateStatusBar();
    QLabel *makeLabel(const QString &text);
    QSlider *makeSlider(int min, int max, int val, Qt::Orientation orient = Qt::Horizontal);

    // Widgets — left panel
    QTabWidget   *tabWidget      = nullptr;

    // Thresholding controls
    QSlider      *sliderThresh   = nullptr;
    QComboBox    *comboThreshMode= nullptr;
    QSlider      *sliderBlock    = nullptr;
    QSlider      *sliderConstC   = nullptr;
    QCheckBox    *checkInvert    = nullptr;

    // Creative Effects controls
    QComboBox    *comboEffect    = nullptr;
    QSlider      *sliderGrain    = nullptr;
    QSlider      *sliderPixel    = nullptr;
    QSlider      *sliderWarmCool = nullptr;
    QComboBox    *comboTint      = nullptr;

    // Interactive Tools controls
    QComboBox    *comboTool      = nullptr;
    QSlider      *sliderBrushSz  = nullptr;
    QComboBox    *comboBrushColor= nullptr;
    QSlider      *sliderBright   = nullptr;
    QSlider      *sliderContrast = nullptr;
    QSlider      *sliderBlur     = nullptr;
    QSlider      *sliderRotate   = nullptr;
    QComboBox    *comboFlip      = nullptr;

    // Image canvas
    QScrollArea  *scrollArea     = nullptr;
    QLabel       *imageLabel     = nullptr;

    // Status
    QLabel       *statusLabel    = nullptr;

    // Actions
    QAction      *actOpen        = nullptr;
    QAction      *actSave        = nullptr;
    QAction      *actUndo        = nullptr;
    QAction      *actRedo        = nullptr;
    QAction      *actReset       = nullptr;

    // State
    QImage        currentImage;
    QImage        originalImage;
    QStack<QImage> undoStack;
    QStack<QImage> redoStack;
    QString       currentFilePath;
};

#endif // MAINWINDOW_H
