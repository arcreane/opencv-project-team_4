#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class StartInterface; }
QT_END_NAMESPACE
class ErosionOperation;

class StartInterface : public QMainWindow {
    Q_OBJECT
public:
    explicit StartInterface(QWidget* parent = nullptr);
    ~StartInterface();
private slots: 

    void goToMorphologyInterface();
    void goToThresholdingInterface();
    void goToCannyEdgeInterface();
    void goToGeometricInterface();
    void goToPanoramaInterface();
    void goToHistogramInterface();
    void goToVideoInterface();
    void goToDeepLearningInterface();
	void goToInteractiveInterface();
	void goToCreativeInterface();

private:
    Ui::StartInterface* ui;
    QImage originalImage;
   

};