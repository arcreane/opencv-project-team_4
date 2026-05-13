#pragma once
#include <QMainWindow>

#include <opencv2/opencv.hpp> 

QT_BEGIN_NAMESPACE
namespace Ui { class DeepLearningWindow; }
QT_END_NAMESPACE

class DeepLearningWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit DeepLearningWindow(QWidget* parent = nullptr);
    ~DeepLearningWindow();

private slots:
    void deepLearningapply();
private:
    Ui::DeepLearningWindow* ui;
    QImage originalImage;
    cv::dnn::Net net;
    std::vector<std::string> classNames;
    void loadClassNames(const std::string& path);
 }; 
