#pragma once
#ifndef IMAGEDROPLABEL_H
#define IMAGEDROPLABEL_H

#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>

class ImageDropLabel : public QLabel {
    Q_OBJECT
public:
    explicit ImageDropLabel(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

#endif