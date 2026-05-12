#pragma once
#include <DilationFilter.h>
#include <ErosionFilter.h>
#include <OpeningFilter.h>
#include <ClosingFilter.h>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class DeepLearningWindow; }
QT_END_NAMESPACE

class DragAndDropWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit DragAndDropWindow(QWidget* parent = nullptr);
    ~DragAndDropWindow();

private slots:
    void deepLearningapply();
private:
    Ui::DeepLearningWindow* ui;
    QImage originalImage;
    

}; 
