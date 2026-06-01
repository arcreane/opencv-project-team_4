#pragma once
#include <DilationFilter.h>
#include <ErosionFilter.h>
#include <OpeningFilter.h>
#include <ClosingFilter.h>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Morphology; }
QT_END_NAMESPACE
class ErosionOperation;

class MorphologyWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MorphologyWindow(QWidget* parent = nullptr);
    ~MorphologyWindow();
    void resizeEvent(QResizeEvent* event) override;
    void repositionWidgets();
    void saveImage();

private slots:
    void createErosionInterface();
    void createDilationInterface();
    void createOpeningInterface();
    void createClosingInterface();
    void applyCurrentFilter();
	void backToStartInterface();

private:
    Ui::Morphology* ui; 
	QImage originalImage;
    ErosionOperation* erosionOp;
    DilationOperation* dilationOp;
	OpeningOperation* openingOp;
	ClosingOperation* closingOp;
	void applyStyles();

};