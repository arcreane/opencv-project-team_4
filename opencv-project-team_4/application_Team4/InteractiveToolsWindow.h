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

class InteractiveToolsWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit InteractiveToolsWindow(QWidget* parent = nullptr);
    ~InteractiveToolsWindow() override;
protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void openImage();
    void saveImage();
    void undoAction();
    void redoAction();
    void resetImage();
    void applyInteractiveTool();
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

    QComboBox* comboTool = nullptr;
    QSlider* sliderBrushSz = nullptr;
    QComboBox* comboBrushColor = nullptr;
    QSlider* sliderBright = nullptr;
    QSlider* sliderContrast = nullptr;
    QSlider* sliderBlur = nullptr;
    QSlider* sliderRotate = nullptr;
    QComboBox* comboFlip = nullptr;

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