#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QAction>
#include <QImage>
#include <QStack>
#include <QString>

class StartInterface;

class ThresholdingWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ThresholdingWindow(QWidget* parent = nullptr);
    ~ThresholdingWindow() override;
protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void openImage();
    void saveImage();
    void undoAction();
    void redoAction();
    void resetImage();
    void applyThreshold();
    void onThreshModeChanged(int idx);
    void onBack();
private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupImageCanvas();
    void setupStatusBar();
    void showImage(const QImage& img);
    void pushUndo();
    void updateStatusBar();
    QSlider* makeSlider(int min, int max, int val, Qt::Orientation orient = Qt::Horizontal);

    StartInterface* m_startInterface = nullptr;

    QSlider* sliderThresh = nullptr;
    QSlider* sliderBlock = nullptr;
    QSlider* sliderConstC = nullptr;
    QComboBox* comboThreshMode = nullptr;
    QCheckBox* checkInvert = nullptr;

    QScrollArea* scrollArea = nullptr;
    QLabel* imageLabel = nullptr;
    QLabel* statusLabel = nullptr;

    QAction* actOpen = nullptr;
    QAction* actSave = nullptr;
    QAction* actUndo = nullptr;
    QAction* actRedo = nullptr;
    QAction* actReset = nullptr;

    QImage         currentImage;
    QImage         originalImage;
    QStack<QImage> undoStack;
    QStack<QImage> redoStack;
    QString        currentFilePath;
};