#pragma once
#include "CannyFilter.h"
#include <QMainWindow>
#include <QImage>



QT_BEGIN_NAMESPACE
namespace Ui { class CannyEdge; }
QT_END_NAMESPACE
class CannyOperation;
class DragDropCannyEdge : public QMainWindow {
    Q_OBJECT
public:
    explicit DragDropCannyEdge(QWidget* parent = nullptr);
    ~DragDropCannyEdge();
private slots:
    void showCannyInterface();
	void hideCannyInterface();
    void onApplyCannyClicked();


private:
    Ui::CannyEdge* ui;
    QImage originalImage;
    CannyOperation* cannyOp;
   

}; 
