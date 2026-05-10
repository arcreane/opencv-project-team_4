#include <QApplication>
#include "DragAndDropWindow.h"  
#include "DragDropCannyEdge.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    DragAndDropWindow window;
    window.setWindowTitle("Morphological Operations");
    //#window.show();
	DragDropCannyEdge cannyWindow;
	cannyWindow.setWindowTitle("Canny Edge Detection");
	cannyWindow.show();
	
    return app.exec();
}