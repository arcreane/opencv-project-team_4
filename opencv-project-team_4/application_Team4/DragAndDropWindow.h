#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Morphology; }
QT_END_NAMESPACE
class ErosionOperation;

class DragAndDropWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit DragAndDropWindow(QWidget* parent = nullptr);
    ~DragAndDropWindow();

private slots:
    void createErosionInterface();
    void createDilationInterface();
    void createOpeningInterface();
    void createClosingInterface();
    void applyCurrentFilter();

private:
    Ui::Morphology* ui; 
	QImage originalImage;
    ErosionOperation* erosionOp;

};