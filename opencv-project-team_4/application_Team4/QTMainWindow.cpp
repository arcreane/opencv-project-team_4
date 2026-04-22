#include "QTMainWindow.h"

QTMainWindow::QTMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

QTMainWindow::~QTMainWindow()
{}

void QTMainWindow::toto()
{
	setWindowTitle("Ca marche");
}
