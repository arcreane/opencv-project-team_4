#include "imagedropLabel.h"
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
    QPixmap pixmap(path);
    setPixmap(pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}