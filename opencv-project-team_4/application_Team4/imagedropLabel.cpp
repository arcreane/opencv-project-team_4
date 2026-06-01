#include "imagedropLabel.h"
#include <qfileinfo.h>
#include <QMimeData>
#include <QPixmap>
#include <QTimer>
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
    if (!validExtensions.contains(QFileInfo(path).suffix().toLower())) {
        setText("Invalid file type.");
        setStyleSheet("border: 2px dashed red; color: red;");
        return;
    }

    QImage image(path);

    if (m_grayscaleOnly && !image.isGrayscale()) {
        setText("Image must be black & white.");
        setStyleSheet("border: 2px dashed red; color: red;");
        return;
    }
    QPoint savedPos = pos();  // sauvegarder position
    QSize  savedSize = size();

    if (m_fixPosition) {
        setStyleSheet("border: 2px dashed gray;");

        setPixmap(QPixmap::fromImage(image).scaled(
            savedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
        QTimer::singleShot(0, this, [this, savedPos, savedSize]() {
            move(savedPos);
            resize(savedSize);
            });
    }
    else {
        setPixmap(QPixmap::fromImage(image).scaled(
            size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
    }

    emit imageDropped(image);
}

