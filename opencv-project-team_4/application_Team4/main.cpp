#include <QApplication>
#include "DragAndDropWindow.h"  

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    DragAndDropWindow window;
    window.show();

    return app.exec();
}