#include "imagedropLabel.h"
#include <qfileinfo.h>
#include <QMimeData>
#include <QPixmap>

ImageDropLabel::ImageDropLabel(QWidget* parent) : QLabel(parent) {
    setAcceptDrops(true);
    setAlignment(Qt::AlignCenter);
    setText("Drop image here");
    setStyleSheet("border: 2px dashed gray;");
}

void ImageDropLabel::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void ImageDropLabel::dropEvent(QDropEvent* event) {
    QString path = event->mimeData()->urls().first().toLocalFile();
    QStringList validExtensions = { "png", "jpg", "jpeg" };
    if(!validExtensions.contains(QFileInfo(path).suffix().toLower())) {
        setText("Invalid file type. Please drop a PNG or JPEG image.");
        setStyleSheet("border: 2px dashed red; color: red; font-weight: bold;");
        return;
    }

    QImage image(path);
    if (!image.isGrayscale()) {
        setText("To use this feature the image should be black & white");
        setStyleSheet("border: 2px dashed red; color: red; font-weight: bold;");
        return;
    }
	setStyleSheet("border: none;");
    QPixmap pixmap(path);
    setPixmap(pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	emit imageDropped(image);
}