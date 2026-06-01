#pragma once
#include "CannyFilter.h"
#include <QMainWindow>
#include <opencv2/opencv.hpp>


QT_BEGIN_NAMESPACE
namespace Ui { class CannyEdge; }
QT_END_NAMESPACE
class CannyOperation;
class DragDropCannyEdge : public QMainWindow {
    Q_OBJECT
public:
    explicit DragDropCannyEdge(QWidget* parent = nullptr);
    ~DragDropCannyEdge();
    void resizeEvent(QResizeEvent* event) override;
    void repositionWidgets();
private slots:
    void showCannyInterface();
    void onApplyCannyClicked();
	void onSliderChanged();
	void saveImage();
    void backToStartInterface();


private:
    Ui::CannyEdge* ui;
    QImage originalImage;
    CannyOperation* cannyOp;
    cv::Mat inputImage;
    bool cannyAppliedOnce = false;
	bool resizeBouton = true;
    void applyStyles();

   
}; 
