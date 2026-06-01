/********************************************************************************
** Form generated from reading UI file 'StartInterface.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STARTINTERFACE_H
#define UI_STARTINTERFACE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StartInterface
{
public:
    QWidget *centralwidget;
    QPushButton *ThresholdingButton;
    QPushButton *PanoramaButton;
    QPushButton *GeometricTransfButton;
    QPushButton *MorphologyButton;
    QPushButton *HistogramButton;
    QPushButton *CannyEdgeButton;
    QPushButton *avoirButton;
    QPushButton *voirButton;
    QPushButton *VideoButton;
    QPushButton *CreativeEffectButton;
    QPushButton *InteractiveButton;
    QPushButton *DeepButton;
    QLabel *title;
    QLabel *logo;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *StartInterface)
    {
        if (StartInterface->objectName().isEmpty())
            StartInterface->setObjectName("StartInterface");
        StartInterface->resize(800, 600);
        centralwidget = new QWidget(StartInterface);
        centralwidget->setObjectName("centralwidget");
        ThresholdingButton = new QPushButton(centralwidget);
        ThresholdingButton->setObjectName("ThresholdingButton");
        ThresholdingButton->setGeometry(QRect(30, 70, 131, 41));
        PanoramaButton = new QPushButton(centralwidget);
        PanoramaButton->setObjectName("PanoramaButton");
        PanoramaButton->setGeometry(QRect(30, 420, 131, 41));
        GeometricTransfButton = new QPushButton(centralwidget);
        GeometricTransfButton->setObjectName("GeometricTransfButton");
        GeometricTransfButton->setGeometry(QRect(30, 350, 131, 41));
        MorphologyButton = new QPushButton(centralwidget);
        MorphologyButton->setObjectName("MorphologyButton");
        MorphologyButton->setGeometry(QRect(30, 280, 131, 41));
        HistogramButton = new QPushButton(centralwidget);
        HistogramButton->setObjectName("HistogramButton");
        HistogramButton->setGeometry(QRect(30, 210, 131, 41));
        CannyEdgeButton = new QPushButton(centralwidget);
        CannyEdgeButton->setObjectName("CannyEdgeButton");
        CannyEdgeButton->setGeometry(QRect(30, 140, 131, 41));
        avoirButton = new QPushButton(centralwidget);
        avoirButton->setObjectName("avoirButton");
        avoirButton->setGeometry(QRect(630, 420, 131, 41));
        voirButton = new QPushButton(centralwidget);
        voirButton->setObjectName("voirButton");
        voirButton->setGeometry(QRect(630, 350, 131, 41));
        VideoButton = new QPushButton(centralwidget);
        VideoButton->setObjectName("VideoButton");
        VideoButton->setGeometry(QRect(630, 280, 131, 41));
        CreativeEffectButton = new QPushButton(centralwidget);
        CreativeEffectButton->setObjectName("CreativeEffectButton");
        CreativeEffectButton->setGeometry(QRect(630, 210, 131, 41));
        InteractiveButton = new QPushButton(centralwidget);
        InteractiveButton->setObjectName("InteractiveButton");
        InteractiveButton->setGeometry(QRect(630, 140, 131, 41));
        DeepButton = new QPushButton(centralwidget);
        DeepButton->setObjectName("DeepButton");
        DeepButton->setGeometry(QRect(630, 70, 131, 41));
        title = new QLabel(centralwidget);
        title->setObjectName("title");
        title->setGeometry(QRect(250, 0, 331, 111));
        QFont font;
        font.setPointSize(33);
        font.setBold(true);
        title->setFont(font);
        title->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        logo = new QLabel(centralwidget);
        logo->setObjectName("logo");
        logo->setGeometry(QRect(270, 140, 271, 261));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Segoe UI")});
        logo->setFont(font1);
        logo->setPixmap(QPixmap(QString::fromUtf8("../../../../../../../Downloads/glimpse_editor_logo_icon_170086.png")));
        logo->setScaledContents(true);
        StartInterface->setCentralWidget(centralwidget);
        menubar = new QMenuBar(StartInterface);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 21));
        StartInterface->setMenuBar(menubar);
        statusbar = new QStatusBar(StartInterface);
        statusbar->setObjectName("statusbar");
        StartInterface->setStatusBar(statusbar);

        retranslateUi(StartInterface);

        QMetaObject::connectSlotsByName(StartInterface);
    } // setupUi

    void retranslateUi(QMainWindow *StartInterface)
    {
        StartInterface->setWindowTitle(QCoreApplication::translate("StartInterface", "MainWindow", nullptr));
        ThresholdingButton->setText(QCoreApplication::translate("StartInterface", "Thresholding", nullptr));
        PanoramaButton->setText(QCoreApplication::translate("StartInterface", "Panorama", nullptr));
        GeometricTransfButton->setText(QCoreApplication::translate("StartInterface", "Geometric Transforms", nullptr));
        MorphologyButton->setText(QCoreApplication::translate("StartInterface", "Morphology", nullptr));
        HistogramButton->setText(QCoreApplication::translate("StartInterface", "Histogram ", nullptr));
        CannyEdgeButton->setText(QCoreApplication::translate("StartInterface", "Canny Edge", nullptr));
        avoirButton->setText(QCoreApplication::translate("StartInterface", "???", nullptr));
        voirButton->setText(QCoreApplication::translate("StartInterface", "???", nullptr));
        VideoButton->setText(QCoreApplication::translate("StartInterface", "Video", nullptr));
        CreativeEffectButton->setText(QCoreApplication::translate("StartInterface", "Creative Effects", nullptr));
        InteractiveButton->setText(QCoreApplication::translate("StartInterface", "Interactive Tools", nullptr));
        DeepButton->setText(QCoreApplication::translate("StartInterface", "Deep Learning", nullptr));
        title->setText(QCoreApplication::translate("StartInterface", "IMAGE EDITOR", nullptr));
        logo->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class StartInterface: public Ui_StartInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STARTINTERFACE_H
