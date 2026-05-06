#ifndef DRAGANDROPWINDOW_H
#define DRAGANDROPWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }  // ← changed to MainWindow
QT_END_NAMESPACE

class DragAndDropWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit DragAndDropWindow(QWidget* parent = nullptr);
    ~DragAndDropWindow();
private:
    Ui::MainWindow* ui;  // ← changed to MainWindow
};

#endif