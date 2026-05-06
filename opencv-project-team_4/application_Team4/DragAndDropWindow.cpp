#include "DragAndDropWindow.h"
#include "ui_DragAndDropWindow.h"

DragAndDropWindow::DragAndDropWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

DragAndDropWindow::~DragAndDropWindow() {
    delete ui;
}