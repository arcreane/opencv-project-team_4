#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QScrollArea>
#include <QAction>
#include <QImage>
#include <QStack>
#include <QString>

class StartInterface;

class CreativeEffectsWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CreativeEffectsWindow(QWidget* parent = nullptr);
    ~CreativeEffectsWindow() override;
protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void openImage();
    void saveImage();
    void undoAction();
    void redoAction();
    void resetImage();
    void applyEffect();
    //void onEffectModeChanged(int idx);
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

    QComboBox* comboEffect = nullptr;
    QSlider* sliderGrain = nullptr;
    QSlider* sliderPixel = nullptr;
    QSlider* sliderWarmCool = nullptr;
    QComboBox* comboTint = nullptr;

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