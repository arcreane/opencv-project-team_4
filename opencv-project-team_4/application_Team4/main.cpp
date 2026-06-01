#include <QApplication>
#include "StartInterface.h"  

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    StartInterface window;
    window.setWindowTitle("ImageEditor");
    window.show();

    return app.exec();
}