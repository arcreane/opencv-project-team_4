#include "GeometricTransform.h"
#include "ui_GeometricTransform.h"

// ---------- ClickableImageLabel ----------

ClickableImageLabel::ClickableImageLabel(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(400, 300);
}

// ---------- GeometricTransformWindow ----------

GeometricTransformWindow::GeometricTransformWindow(QWidget* startInterface, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::GeometricTransform)
    , m_startInterface(startInterface)
{
    ui->setupUi(this);
}

GeometricTransformWindow::~GeometricTransformWindow()
{
    delete ui;
}
