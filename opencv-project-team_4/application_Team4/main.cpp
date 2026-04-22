#include <QTMainWindow.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QTMainWindow window;
    window.resize(320, 240);
    window.show();
    window.setWindowTitle(
        QApplication::translate("toplevel", "Top-level widget"));
    return app.exec();
}