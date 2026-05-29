#include "VideoProcessing.h"
#include "ui_VideoProcessing.h"
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video.hpp>

// ============================================================
// Construction
// ============================================================

VideoProcessingWindow::VideoProcessingWindow(QWidget* startInterface, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoProcessing)
    , m_startInterface(startInterface)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    applyStyles();

    m_exportTimer    = new QTimer(this); m_exportTimer->setInterval(30);
    m_flowTimer      = new QTimer(this); m_flowTimer->setInterval(60);
    m_camTimer       = new QTimer(this); m_camTimer->setInterval(30);
    m_timelapseTimer = new QTimer(this); m_timelapseTimer->setInterval(30);
    m_stabTimer      = new QTimer(this); m_stabTimer->setInterval(20);
    m_stabExportTimer= new QTimer(this); m_stabExportTimer->setInterval(30);

    connect(m_exportTimer,     &QTimer::timeout, this, &VideoProcessingWindow::onExportTimerTick);
    connect(m_flowTimer,       &QTimer::timeout, this, &VideoProcessingWindow::onFlowTimerTick);
    connect(m_camTimer,        &QTimer::timeout, this, &VideoProcessingWindow::onCamTimerTick);
    connect(m_timelapseTimer,  &QTimer::timeout, this, &VideoProcessingWindow::onTimelapseExportTick);
    connect(m_stabTimer,       &QTimer::timeout, this, &VideoProcessingWindow::onStabTimerTick);
    connect(m_stabExportTimer, &QTimer::timeout, this, &VideoProcessingWindow::onStabExportTick);

    // Tab 1
    connect(ui->loadFilterButton,   &QPushButton::clicked, this, &VideoProcessingWindow::onLoadVideoFilter);
    connect(ui->filterSlider,       &QSlider::valueChanged, this, &VideoProcessingWindow::onFilterSliderChanged);
    connect(ui->filterCombo,        &QComboBox::currentIndexChanged, this,
            [this](int){ onFilterSliderChanged(ui->filterSlider->value()); });
    connect(ui->exportFilterButton, &QPushButton::clicked, this, &VideoProcessingWindow::onExportFilter);
    // Tab 2
    connect(ui->loadFlowButton,   &QPushButton::clicked,  this, &VideoProcessingWindow::onLoadVideoFlow);
    connect(ui->playFlowButton,   &QPushButton::clicked,  this, &VideoProcessingWindow::onToggleFlowPlay);
    connect(ui->flowSpeedSlider,  &QSlider::valueChanged, this, &VideoProcessingWindow::onFlowSpeedChanged);
    connect(ui->flowSlider,       &QSlider::valueChanged, this, &VideoProcessingWindow::onFlowSliderChanged);
    connect(ui->flowCombo, &QComboBox::currentIndexChanged, this,
            [this](int){ m_flowPoints.clear(); m_prevGrayFlow = cv::Mat(); });
    // Tab 3
    connect(ui->startCamButton,     &QPushButton::clicked,  this, &VideoProcessingWindow::onStartCam);
    connect(ui->stopCamButton,      &QPushButton::clicked,  this, &VideoProcessingWindow::onStopCam);
    connect(ui->motionThreshSlider, &QSlider::valueChanged, this, &VideoProcessingWindow::onMotionThreshChanged);
    // Tab 4
    connect(ui->loadTimelapseButton,   &QPushButton::clicked,  this, &VideoProcessingWindow::onLoadVideoTimelapse);
    connect(ui->timelapseSlider,       &QSlider::valueChanged, this, &VideoProcessingWindow::onTimelapseSliderChanged);
    connect(ui->exportTimelapseButton, &QPushButton::clicked,  this, &VideoProcessingWindow::onExportTimelapse);
    // Tab 5
    connect(ui->loadStabButton,  &QPushButton::clicked,  this, &VideoProcessingWindow::onLoadVideoStab);
    connect(ui->stabilizeButton, &QPushButton::clicked,  this, &VideoProcessingWindow::onStabilize);
    connect(ui->stabSlider,      &QSlider::valueChanged, this, &VideoProcessingWindow::onStabSliderChanged);
    connect(ui->exportStabButton,&QPushButton::clicked,  this, &VideoProcessingWindow::onExportStab);
    // Common
    connect(ui->backButton, &QPushButton::clicked, this, &VideoProcessingWindow::onBack);
}

VideoProcessingWindow::~VideoProcessingWindow()
{
    for (auto* t : {m_exportTimer, m_flowTimer, m_camTimer,
                    m_timelapseTimer, m_stabTimer, m_stabExportTimer}) t->stop();
    m_filterWriter.release(); m_motionWriter.release();
    m_timelapseWriter.release(); m_stabWriter.release();
    m_filterCap.release(); m_flowCap.release();
    m_webcam.release(); m_timelapseCap.release(); m_stabCap.release();
    delete ui;
}

// ============================================================
// Tab 1 — Per-Frame Filter
// ============================================================

void VideoProcessingWindow::onLoadVideoFilter()
{
    QString path = QFileDialog::getOpenFileName(this, "Ouvrir une vidéo", {},
        "Vidéos (*.mp4 *.avi *.mkv *.mov *.wmv)");
    if (path.isEmpty()) return;
    m_filterCap.release(); m_filterCap.open(path.toStdString());
    if (!m_filterCap.isOpened()) { setStatus("Erreur : impossible d'ouvrir la vidéo.", StatusType::Error); return; }
    m_filterTotal = (int)m_filterCap.get(cv::CAP_PROP_FRAME_COUNT);
    ui->filterSlider->setRange(0, qMax(0, m_filterTotal-1));
    ui->filterSlider->setValue(0);
    ui->exportFilterButton->setEnabled(true);
    onFilterSliderChanged(0);
    setStatus(QString("Vidéo : %1×%2  |  %3 fps  |  %4 frames")
              .arg((int)m_filterCap.get(cv::CAP_PROP_FRAME_WIDTH))
              .arg((int)m_filterCap.get(cv::CAP_PROP_FRAME_HEIGHT))
              .arg((int)m_filterCap.get(cv::CAP_PROP_FPS)).arg(m_filterTotal));
}

void VideoProcessingWindow::onFilterSliderChanged(int value)
{
    cv::Mat frame = grabFrame(value);
    if (frame.empty()) return;
    cv::Mat filtered = applyFilter(frame, ui->filterCombo->currentText());
    QSize sz = ui->filterPreviewLabel->size(); QPixmap px = matToPixmap(filtered);
    ui->filterPreviewLabel->setPixmap(sz.width()>10 ? px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation) : px);
    ui->frameCountLabel->setText(QString("%1 / %2").arg(value).arg(qMax(0,m_filterTotal-1)));
}

void VideoProcessingWindow::onExportFilter()
{
    if (!m_filterCap.isOpened()) return;
    QString path = QFileDialog::getSaveFileName(this,"Exporter","output.mp4","MP4 (*.mp4);;AVI (*.avi)");
    if (path.isEmpty()) return;
    double fps=(int)m_filterCap.get(cv::CAP_PROP_FPS);
    int w=(int)m_filterCap.get(cv::CAP_PROP_FRAME_WIDTH), h=(int)m_filterCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fc=path.endsWith(".mp4",Qt::CaseInsensitive)?cv::VideoWriter::fourcc('m','p','4','v'):cv::VideoWriter::fourcc('M','J','P','G');
    if (!m_filterWriter.open(path.toStdString(),fc,fps,cv::Size(w,h))) { setStatus("Erreur : impossible de créer le fichier.",StatusType::Error); return; }
    m_filterCap.set(cv::CAP_PROP_POS_FRAMES,0); m_exportCurrent=0;
    ui->filterProgressBar->setRange(0,m_filterTotal); ui->filterProgressBar->setValue(0);
    ui->filterProgressBar->show(); ui->exportFilterButton->setEnabled(false);
    m_exportTimer->start(); setStatus("Export filtre en cours...");
}

void VideoProcessingWindow::onExportTimerTick()
{
    const QString filter=ui->filterCombo->currentText();
    for (int i=0;i<5&&m_exportCurrent<m_filterTotal;++i) {
        cv::Mat frame; if (!m_filterCap.read(frame)) {m_exportCurrent=m_filterTotal; break;}
        m_filterWriter.write(applyFilter(frame,filter)); ++m_exportCurrent;
    }
    ui->filterProgressBar->setValue(m_exportCurrent);
    if (m_exportCurrent>=m_filterTotal) {
        m_exportTimer->stop(); m_filterWriter.release();
        ui->filterProgressBar->hide(); ui->exportFilterButton->setEnabled(true);
        setStatus("Export terminé.",StatusType::Success);
    }
}

// ============================================================
// Tab 2 — Optical Flow
// ============================================================

void VideoProcessingWindow::onLoadVideoFlow()
{
    QString path=QFileDialog::getOpenFileName(this,"Ouvrir une vidéo",{},"Vidéos (*.mp4 *.avi *.mkv *.mov *.wmv)");
    if (path.isEmpty()) return;
    m_flowCap.release(); m_flowCap.open(path.toStdString());
    if (!m_flowCap.isOpened()) { setStatus("Erreur.",StatusType::Error); return; }
    m_flowTotal=(int)m_flowCap.get(cv::CAP_PROP_FRAME_COUNT); m_flowFrameIdx=0;
    m_prevGrayFlow=cv::Mat(); m_flowPoints.clear();
    ui->flowSlider->setRange(0,qMax(0,m_flowTotal-1));
    ui->flowSlider->blockSignals(true); ui->flowSlider->setValue(0); ui->flowSlider->blockSignals(false);
    ui->playFlowButton->setEnabled(true);
    cv::Mat first; m_flowCap.read(first);
    if (!first.empty()) { QSize sz=ui->flowPreviewLabel->size(); QPixmap px=matToPixmap(first);
        ui->flowPreviewLabel->setPixmap(sz.width()>10?px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation):px); }
    m_flowCap.set(cv::CAP_PROP_POS_FRAMES,0);
    ui->flowCountLabel->setText(QString("0 / %1").arg(m_flowTotal-1));
    setStatus(QString("Vidéo : %1×%2  |  %3 fps  |  %4 frames — cliquez Play")
              .arg((int)m_flowCap.get(cv::CAP_PROP_FRAME_WIDTH)).arg((int)m_flowCap.get(cv::CAP_PROP_FRAME_HEIGHT))
              .arg((int)m_flowCap.get(cv::CAP_PROP_FPS)).arg(m_flowTotal));
}

void VideoProcessingWindow::onToggleFlowPlay()
{
    if (!m_flowCap.isOpened()) return;
    m_flowPlaying=!m_flowPlaying;
    if (m_flowPlaying) {
        m_flowCap.set(cv::CAP_PROP_POS_FRAMES,(double)m_flowFrameIdx);
        m_prevGrayFlow=cv::Mat(); m_flowPoints.clear();
        m_flowTimer->start(); ui->playFlowButton->setText("Pause");
    } else { m_flowTimer->stop(); ui->playFlowButton->setText("Play"); setStatus("Pausé."); }
}

void VideoProcessingWindow::onFlowSpeedChanged(int v) { m_flowTimer->setInterval(qMax(16,210-v*20)); }

void VideoProcessingWindow::onFlowSliderChanged(int value)
{
    if (!m_flowCap.isOpened()) return;
    if (m_flowPlaying) { m_flowTimer->stop(); m_flowPlaying=false; ui->playFlowButton->setText("Play"); }
    m_flowFrameIdx=value; m_prevGrayFlow=cv::Mat(); m_flowPoints.clear();
    m_flowCap.set(cv::CAP_PROP_POS_FRAMES,(double)value);
    cv::Mat frame; m_flowCap.read(frame);
    if (!frame.empty()) { QSize sz=ui->flowPreviewLabel->size(); QPixmap px=matToPixmap(frame);
        ui->flowPreviewLabel->setPixmap(sz.width()>10?px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation):px); }
    ui->flowCountLabel->setText(QString("%1 / %2").arg(value).arg(m_flowTotal-1));
}

void VideoProcessingWindow::onFlowTimerTick()
{
    cv::Mat curr; if (!m_flowCap.read(curr)||curr.empty()) {
        m_flowTimer->stop(); m_flowPlaying=false; ui->playFlowButton->setText("Play");
        m_flowFrameIdx=0; setStatus("Lecture terminée.",StatusType::Success); return;
    }
    cv::Mat result=m_prevGrayFlow.empty()?curr.clone()
        :(ui->flowCombo->currentText().contains("Farneback")
          ?computeFarnebackFlow(m_prevGrayFlow,curr)
          :computeLucasKanadeFlow(m_prevGrayFlow,curr));
    cv::cvtColor(curr,m_prevGrayFlow,cv::COLOR_BGR2GRAY); m_flowFrameIdx++;
    QSize sz=ui->flowPreviewLabel->size(); QPixmap px=matToPixmap(result);
    ui->flowPreviewLabel->setPixmap(sz.width()>10?px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation):px);
    ui->flowSlider->blockSignals(true); ui->flowSlider->setValue(m_flowFrameIdx); ui->flowSlider->blockSignals(false);
    ui->flowCountLabel->setText(QString("%1 / %2").arg(m_flowFrameIdx).arg(m_flowTotal-1));
    if (m_flowFrameIdx>=m_flowTotal-1) { m_flowTimer->stop(); m_flowPlaying=false; ui->playFlowButton->setText("Play"); setStatus("Terminé.",StatusType::Success); }
}

cv::Mat VideoProcessingWindow::computeFarnebackFlow(const cv::Mat& prevGray, const cv::Mat& curr)
{
    cv::Mat currGray; if (curr.channels()==1) currGray=curr; else cv::cvtColor(curr,currGray,cv::COLOR_BGR2GRAY);
    cv::Mat flow; cv::calcOpticalFlowFarneback(prevGray,currGray,flow,0.5,3,15,3,5,1.2,0);
    std::vector<cv::Mat> parts(2); cv::split(flow,parts);
    cv::Mat mag,ang,magN; cv::cartToPolar(parts[0],parts[1],mag,ang,true);
    cv::normalize(mag,magN,0,255,cv::NORM_MINMAX);
    cv::Mat h,s,v; ang.convertTo(h,CV_8U,0.5); s=cv::Mat::ones(ang.size(),CV_8U)*255; magN.convertTo(v,CV_8U);
    cv::Mat hsv,bgr; cv::merge(std::vector<cv::Mat>{h,s,v},hsv); cv::cvtColor(hsv,bgr,cv::COLOR_HSV2BGR);
    return bgr;
}

cv::Mat VideoProcessingWindow::computeLucasKanadeFlow(const cv::Mat& prevGray, const cv::Mat& curr)
{
    cv::Mat currGray; if (curr.channels()==1) currGray=curr; else cv::cvtColor(curr,currGray,cv::COLOR_BGR2GRAY);
    cv::Mat draw=curr.channels()==1?cv::Mat():curr.clone(); if (draw.empty()) cv::cvtColor(curr,draw,cv::COLOR_GRAY2BGR);
    if (m_flowPoints.empty()) cv::goodFeaturesToTrack(prevGray,m_flowPoints,200,0.01,10);
    if (m_flowPoints.empty()) return draw;
    std::vector<cv::Point2f> nxt; std::vector<uchar> st; std::vector<float> er;
    cv::calcOpticalFlowPyrLK(prevGray,currGray,m_flowPoints,nxt,st,er,cv::Size(21,21),3);
    std::vector<cv::Point2f> good;
    for (size_t i=0;i<m_flowPoints.size();++i) {
        if (!st[i]) continue;
        cv::arrowedLine(draw,m_flowPoints[i],nxt[i],cv::Scalar(0,255,0),2,cv::LINE_AA,0,0.3);
        cv::circle(draw,nxt[i],3,cv::Scalar(0,80,255),-1); good.push_back(nxt[i]);
    }
    m_flowPoints=good; if (m_flowPoints.size()<20) m_flowPoints.clear();
    return draw;
}

// ============================================================
// Tab 3 — Motion-Triggered Recording
// ============================================================

void VideoProcessingWindow::onStartCam()
{
    m_webcam.release(); m_webcam.open(0);
    if (!m_webcam.isOpened()) { setStatus("Erreur : impossible d'ouvrir la webcam.",StatusType::Error); return; }
    m_prevFrameGray=cv::Mat(); m_recording=false; m_noMotionFrames=0;
    ui->startCamButton->setEnabled(false); ui->stopCamButton->setEnabled(true);
    m_camTimer->start(); setStatus("Webcam démarrée — surveillance active.");
}

void VideoProcessingWindow::onStopCam()
{
    m_camTimer->stop(); if (m_recording) { m_motionWriter.release(); m_recording=false; }
    m_webcam.release(); m_prevFrameGray=cv::Mat();
    ui->startCamButton->setEnabled(true); ui->stopCamButton->setEnabled(false);
    ui->motionIndicatorLabel->setText("● Aucun mouvement");
    ui->motionIndicatorLabel->setStyleSheet("color:#48d890;font-family:'Segoe UI';font-size:13px;font-weight:bold;");
    ui->camPreviewLabel->clear(); ui->camPreviewLabel->setText("Démarrez la webcam pour afficher le flux");
    setStatus("Webcam arrêtée.");
}

void VideoProcessingWindow::onMotionThreshChanged(int v) { ui->threshValueLabel->setText(QString::number(v)); }

void VideoProcessingWindow::onCamTimerTick()
{
    cv::Mat frame; if (!m_webcam.read(frame)||frame.empty()) return;
    cv::Mat cg; cv::cvtColor(frame,cg,cv::COLOR_BGR2GRAY); cv::GaussianBlur(cg,cg,cv::Size(5,5),0);
    bool motionDetected=false;
    if (!m_prevFrameGray.empty()) {
        cv::Mat diff,thresh; cv::absdiff(m_prevFrameGray,cg,diff); cv::threshold(diff,thresh,30,255,cv::THRESH_BINARY);
        double pct=(double)cv::countNonZero(thresh)/(frame.rows*frame.cols)*100.0;
        double trig=(101-ui->motionThreshSlider->value())/10.0;
        motionDetected=pct>trig;
        if (motionDetected) {
            m_noMotionFrames=0;
            if (!m_recording) {
                QString ts=QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                m_motionRecordPath=QDir::currentPath()+"/motion_"+ts+".avi";
                m_motionWriter.open(m_motionRecordPath.toStdString(),cv::VideoWriter::fourcc('M','J','P','G'),30.0,cv::Size(frame.cols,frame.rows));
                m_recording=true; ui->recPathLabel->setText("Enregistrement : "+m_motionRecordPath);
                setStatus("Mouvement — enregistrement démarré.",StatusType::Success);
            }
        } else if (++m_noMotionFrames>=100&&m_recording) {
            m_motionWriter.release(); m_recording=false;
            setStatus("Enregistrement terminé : "+m_motionRecordPath,StatusType::Success);
        }
    }
    m_prevFrameGray=cg;
    if (m_recording&&m_motionWriter.isOpened()) m_motionWriter.write(frame);
    ui->motionIndicatorLabel->setText(motionDetected?"● Mouvement détecté !":m_recording?"● Attente fin...":"● Aucun mouvement");
    ui->motionIndicatorLabel->setStyleSheet(motionDetected
        ?"color:#ff5f5f;font-family:'Segoe UI';font-size:13px;font-weight:bold;"
        :m_recording?"color:#ffaa44;font-family:'Segoe UI';font-size:13px;font-weight:bold;"
        :"color:#48d890;font-family:'Segoe UI';font-size:13px;font-weight:bold;");
    QSize sz=ui->camPreviewLabel->size(); QPixmap px=matToPixmap(frame);
    ui->camPreviewLabel->setPixmap(sz.width()>10?px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation):px);
}

// ============================================================
// Tab 4 — Time-lapse & Slow-motion
// ============================================================

void VideoProcessingWindow::onLoadVideoTimelapse()
{
    QString path=QFileDialog::getOpenFileName(this,"Ouvrir une vidéo",{},"Vidéos (*.mp4 *.avi *.mkv *.mov *.wmv)");
    if (path.isEmpty()) return;
    m_timelapseCap.release(); m_timelapseCap.open(path.toStdString());
    if (!m_timelapseCap.isOpened()) { setStatus("Erreur.",StatusType::Error); return; }
    m_timelapseTotal=(int)m_timelapseCap.get(cv::CAP_PROP_FRAME_COUNT);
    ui->timelapseSlider->setRange(0,qMax(0,m_timelapseTotal-1)); ui->timelapseSlider->setValue(0);
    ui->exportTimelapseButton->setEnabled(true); onTimelapseSliderChanged(0);
    setStatus(QString("Vidéo : %1×%2  |  %3 fps  |  %4 frames")
              .arg((int)m_timelapseCap.get(cv::CAP_PROP_FRAME_WIDTH)).arg((int)m_timelapseCap.get(cv::CAP_PROP_FRAME_HEIGHT))
              .arg((int)m_timelapseCap.get(cv::CAP_PROP_FPS)).arg(m_timelapseTotal));
}

void VideoProcessingWindow::onTimelapseSliderChanged(int v)
{
    if (!m_timelapseCap.isOpened()) return;
    cv::Mat f=grabTimelapsFrame(v); if (f.empty()) return;
    QSize sz=ui->timelapsePreviewLabel->size(); QPixmap px=matToPixmap(f);
    ui->timelapsePreviewLabel->setPixmap(sz.width()>10?px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation):px);
    ui->timelapseCountLabel->setText(QString("%1 / %2").arg(v).arg(qMax(0,m_timelapseTotal-1)));
}

void VideoProcessingWindow::onExportTimelapse()
{
    if (!m_timelapseCap.isOpened()) return;
    QString path=QFileDialog::getSaveFileName(this,"Exporter","output.mp4","MP4 (*.mp4);;AVI (*.avi)");
    if (path.isEmpty()) return;
    double origFps=m_timelapseCap.get(cv::CAP_PROP_FPS);
    int w=(int)m_timelapseCap.get(cv::CAP_PROP_FRAME_WIDTH), h=(int)m_timelapseCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int factor=2<<ui->timelapseFactorCombo->currentIndex();
    bool isTL=(ui->timelapseModeCombo->currentIndex()==0);
    double outFps=isTL?origFps:qMax(1.0,origFps/factor);
    int fc=path.endsWith(".mp4",Qt::CaseInsensitive)?cv::VideoWriter::fourcc('m','p','4','v'):cv::VideoWriter::fourcc('M','J','P','G');
    if (!m_timelapseWriter.open(path.toStdString(),fc,outFps,cv::Size(w,h))) { setStatus("Erreur.",StatusType::Error); return; }
    m_timelapseCap.set(cv::CAP_PROP_POS_FRAMES,0); m_timelapseExportIdx=0;
    ui->timelapseProgressBar->setRange(0,m_timelapseTotal); ui->timelapseProgressBar->setValue(0);
    ui->timelapseProgressBar->show(); ui->exportTimelapseButton->setEnabled(false);
    m_timelapseTimer->start();
    setStatus(isTL?QString("Time-lapse ×%1 en cours...").arg(factor):QString("Slow-motion ×1/%1 en cours...").arg(factor));
}

void VideoProcessingWindow::onTimelapseExportTick()
{
    bool isTL=(ui->timelapseModeCombo->currentIndex()==0);
    int factor=2<<ui->timelapseFactorCombo->currentIndex();
    for (int i=0;i<5&&m_timelapseExportIdx<m_timelapseTotal;++i) {
        cv::Mat frame; if (!m_timelapseCap.read(frame)) {m_timelapseExportIdx=m_timelapseTotal; break;}
        if (!isTL||m_timelapseExportIdx%factor==0) m_timelapseWriter.write(frame);
        ++m_timelapseExportIdx;
    }
    ui->timelapseProgressBar->setValue(m_timelapseExportIdx);
    if (m_timelapseExportIdx>=m_timelapseTotal) {
        m_timelapseTimer->stop(); m_timelapseWriter.release();
        ui->timelapseProgressBar->hide(); ui->exportTimelapseButton->setEnabled(true);
        setStatus("Export terminé.",StatusType::Success);
    }
}

// ============================================================
// Tab 5 — Video Stabilization
// ============================================================

void VideoProcessingWindow::onLoadVideoStab()
{
    QString path=QFileDialog::getOpenFileName(this,"Ouvrir une vidéo",{},"Vidéos (*.mp4 *.avi *.mkv *.mov *.wmv)");
    if (path.isEmpty()) return;
    m_stabCap.release(); m_stabCap.open(path.toStdString());
    if (!m_stabCap.isOpened()) { setStatus("Erreur : impossible d'ouvrir la vidéo.",StatusType::Error); return; }
    m_stabTotal=(int)m_stabCap.get(cv::CAP_PROP_FRAME_COUNT);
    m_stabPhase=0; m_stabTransforms.clear(); m_stabTrajectory.clear(); m_stabSmoothed.clear();
    ui->stabSlider->setRange(0,qMax(0,m_stabTotal-1)); ui->stabSlider->setValue(0);
    ui->stabilizeButton->setEnabled(true); ui->exportStabButton->setEnabled(false);
    cv::Mat first; m_stabCap.read(first);
    if (!first.empty()) {
        QSize sz=ui->stabOrigLabel->size(); QPixmap px=matToPixmap(first);
        ui->stabOrigLabel->setPixmap(sz.width()>10?px.scaled(sz,Qt::KeepAspectRatio,Qt::SmoothTransformation):px);
        ui->stabResultLabel->setText("Cliquez sur Stabiliser");
    }
    m_stabCap.set(cv::CAP_PROP_POS_FRAMES,0);
    ui->stabCountLabel->setText(QString("0 / %1").arg(m_stabTotal-1));
    setStatus(QString("Vidéo : %1×%2  |  %3 fps  |  %4 frames — cliquez Stabiliser")
              .arg((int)m_stabCap.get(cv::CAP_PROP_FRAME_WIDTH)).arg((int)m_stabCap.get(cv::CAP_PROP_FRAME_HEIGHT))
              .arg((int)m_stabCap.get(cv::CAP_PROP_FPS)).arg(m_stabTotal));
}

void VideoProcessingWindow::onStabilize()
{
    if (!m_stabCap.isOpened()) return;
    m_stabTransforms.clear(); m_stabTrajectory.clear(); m_stabSmoothed.clear();
    m_stabTransforms.reserve(m_stabTotal);
    m_stabPhase=1; m_stabProcessIdx=0; m_stabPrevGray=cv::Mat();
    m_stabCap.set(cv::CAP_PROP_POS_FRAMES,0);
    ui->stabProgressBar->setRange(0,m_stabTotal); ui->stabProgressBar->setValue(0);
    ui->stabProgressBar->show(); ui->stabilizeButton->setEnabled(false); ui->exportStabButton->setEnabled(false);
    m_stabTimer->start();
    setStatus("Phase 1 : calcul des transformations en cours...");
}

void VideoProcessingWindow::onStabTimerTick()
{
    const int batch=3;
    for (int i=0;i<batch&&m_stabProcessIdx<m_stabTotal;++i) {
        cv::Mat frame; if (!m_stabCap.read(frame)) { m_stabProcessIdx=m_stabTotal; break; }
        cv::Mat gray; cv::cvtColor(frame,gray,cv::COLOR_BGR2GRAY);
        if (!m_stabPrevGray.empty())
            m_stabTransforms.push_back(computeStabTransform(m_stabPrevGray,gray));
        m_stabPrevGray=gray; ++m_stabProcessIdx;
    }
    ui->stabProgressBar->setValue(m_stabProcessIdx);
    if (m_stabProcessIdx>=m_stabTotal) {
        m_stabTimer->stop();
        computeTrajectoryAndSmooth();
        m_stabPhase=2;
        ui->stabProgressBar->hide();
        ui->stabilizeButton->setEnabled(true); ui->exportStabButton->setEnabled(true);
        onStabSliderChanged(0);
        setStatus(QString("Stabilisation prête — %1 transformations calculées.").arg(m_stabTransforms.size()),StatusType::Success);
    }
}

void VideoProcessingWindow::onStabSliderChanged(int value)
{
    if (!m_stabCap.isOpened()) return;
    m_stabCap.set(cv::CAP_PROP_POS_FRAMES,(double)value);
    cv::Mat frame; m_stabCap.read(frame); if (frame.empty()) return;

    // Original
    QSize sz1=ui->stabOrigLabel->size(); QPixmap pxO=matToPixmap(frame);
    ui->stabOrigLabel->setPixmap(sz1.width()>10?pxO.scaled(sz1,Qt::KeepAspectRatio,Qt::SmoothTransformation):pxO);

    // Stabilized (if ready)
    if (m_stabPhase==2&&value<(int)m_stabSmoothed.size()) {
        cv::Mat stab=applyStabCorrection(frame,value);
        QSize sz2=ui->stabResultLabel->size(); QPixmap pxS=matToPixmap(stab);
        ui->stabResultLabel->setPixmap(sz2.width()>10?pxS.scaled(sz2,Qt::KeepAspectRatio,Qt::SmoothTransformation):pxS);
    }
    ui->stabCountLabel->setText(QString("%1 / %2").arg(value).arg(m_stabTotal-1));
}

void VideoProcessingWindow::onExportStab()
{
    if (!m_stabCap.isOpened()||m_stabPhase!=2) return;
    QString path=QFileDialog::getSaveFileName(this,"Exporter vidéo stabilisée","stabilized.mp4","MP4 (*.mp4);;AVI (*.avi)");
    if (path.isEmpty()) return;
    double fps=m_stabCap.get(cv::CAP_PROP_FPS);
    int w=(int)m_stabCap.get(cv::CAP_PROP_FRAME_WIDTH), h=(int)m_stabCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fc=path.endsWith(".mp4",Qt::CaseInsensitive)?cv::VideoWriter::fourcc('m','p','4','v'):cv::VideoWriter::fourcc('M','J','P','G');
    if (!m_stabWriter.open(path.toStdString(),fc,fps,cv::Size(w,h))) { setStatus("Erreur.",StatusType::Error); return; }
    m_stabCap.set(cv::CAP_PROP_POS_FRAMES,0); m_stabExportIdx=0;
    ui->stabProgressBar->setRange(0,m_stabTotal); ui->stabProgressBar->setValue(0);
    ui->stabProgressBar->show(); ui->exportStabButton->setEnabled(false);
    m_stabExportTimer->start(); setStatus("Export vidéo stabilisée en cours...");
}

void VideoProcessingWindow::onStabExportTick()
{
    for (int i=0;i<3&&m_stabExportIdx<m_stabTotal;++i) {
        cv::Mat frame; if (!m_stabCap.read(frame)) {m_stabExportIdx=m_stabTotal; break;}
        m_stabWriter.write(applyStabCorrection(frame,m_stabExportIdx));
        ++m_stabExportIdx;
    }
    ui->stabProgressBar->setValue(m_stabExportIdx);
    if (m_stabExportIdx>=m_stabTotal) {
        m_stabExportTimer->stop(); m_stabWriter.release();
        ui->stabProgressBar->hide(); ui->exportStabButton->setEnabled(true);
        setStatus("Export terminé.",StatusType::Success);
    }
}

// ── Stab helpers ─────────────────────────────────────────────

VideoProcessingWindow::StabT VideoProcessingWindow::computeStabTransform(
    const cv::Mat& prev, const cv::Mat& curr)
{
    std::vector<cv::Point2f> prevPts;
    cv::goodFeaturesToTrack(prev,prevPts,200,0.01,30);
    if (prevPts.empty()) return {};
    std::vector<cv::Point2f> currPts; std::vector<uchar> st; std::vector<float> er;
    cv::calcOpticalFlowPyrLK(prev,curr,prevPts,currPts,st,er);
    std::vector<cv::Point2f> gp,gc;
    for (size_t i=0;i<prevPts.size();++i) if (st[i]) { gp.push_back(prevPts[i]); gc.push_back(currPts[i]); }
    if (gp.size()<4) return {};
    cv::Mat M=cv::estimateAffinePartial2D(gp,gc);
    if (M.empty()) return {};
    return { M.at<double>(0,2), M.at<double>(1,2),
             std::atan2(M.at<double>(1,0),M.at<double>(0,0)) };
}

void VideoProcessingWindow::computeTrajectoryAndSmooth()
{
    int n=(int)m_stabTransforms.size()+1;
    m_stabTrajectory.resize(n); m_stabSmoothed.resize(n);
    m_stabTrajectory[0]={0,0,0};
    for (int i=1;i<n;++i) {
        m_stabTrajectory[i].dx=m_stabTrajectory[i-1].dx+m_stabTransforms[i-1].dx;
        m_stabTrajectory[i].dy=m_stabTrajectory[i-1].dy+m_stabTransforms[i-1].dy;
        m_stabTrajectory[i].da=m_stabTrajectory[i-1].da+m_stabTransforms[i-1].da;
    }
    const int R=15; // 30-frame window
    for (int i=0;i<n;++i) {
        double sdx=0,sdy=0,sda=0; int cnt=0;
        for (int j=qMax(0,i-R);j<=qMin(n-1,i+R);++j) {
            sdx+=m_stabTrajectory[j].dx; sdy+=m_stabTrajectory[j].dy;
            sda+=m_stabTrajectory[j].da; ++cnt;
        }
        m_stabSmoothed[i]={sdx/cnt,sdy/cnt,sda/cnt};
    }
}

cv::Mat VideoProcessingWindow::applyStabCorrection(const cv::Mat& frame, int idx)
{
    if (idx>=(int)m_stabSmoothed.size()) return frame.clone();
    double dx=m_stabSmoothed[idx].dx-m_stabTrajectory[idx].dx;
    double dy=m_stabSmoothed[idx].dy-m_stabTrajectory[idx].dy;
    double da=m_stabSmoothed[idx].da-m_stabTrajectory[idx].da;
    double ca=std::cos(da), sa=std::sin(da);
    cv::Mat M=(cv::Mat_<double>(2,3)<<ca,-sa,dx,sa,ca,dy);
    cv::Mat result;
    cv::warpAffine(frame,result,M,frame.size(),cv::INTER_LINEAR,cv::BORDER_REFLECT_101);
    return result;
}

// ============================================================
// Common helpers
// ============================================================

void VideoProcessingWindow::onBack()
{
    if (m_startInterface) m_startInterface->show();
    close();
}

cv::Mat VideoProcessingWindow::grabFrame(int index)
{
    if (!m_filterCap.isOpened()) return {};
    m_filterCap.set(cv::CAP_PROP_POS_FRAMES,(double)index);
    cv::Mat f; m_filterCap.read(f); return f;
}

cv::Mat VideoProcessingWindow::grabTimelapsFrame(int index)
{
    if (!m_timelapseCap.isOpened()) return {};
    m_timelapseCap.set(cv::CAP_PROP_POS_FRAMES,(double)index);
    cv::Mat f; m_timelapseCap.read(f); return f;
}

QPixmap VideoProcessingWindow::matToPixmap(const cv::Mat& mat)
{
    if (mat.empty()) return {};
    cv::Mat rgb; if (mat.channels()==1) cv::cvtColor(mat,rgb,cv::COLOR_GRAY2RGB); else cv::cvtColor(mat,rgb,cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(QImage(rgb.data,rgb.cols,rgb.rows,(int)rgb.step,QImage::Format_RGB888).copy());
}

cv::Mat VideoProcessingWindow::applyFilter(const cv::Mat& frame, const QString& f)
{
    if (frame.empty()) return {};
    cv::Mat r;
    if (f=="Grayscale") { cv::cvtColor(frame,r,cv::COLOR_BGR2GRAY); cv::cvtColor(r,r,cv::COLOR_GRAY2BGR); }
    else if (f=="Blur") cv::GaussianBlur(frame,r,cv::Size(15,15),0);
    else if (f=="Canny Edge") { cv::Mat g; cv::cvtColor(frame,g,cv::COLOR_BGR2GRAY); cv::Canny(g,r,50,150); cv::cvtColor(r,r,cv::COLOR_GRAY2BGR); }
    else if (f=="Sepia") {
        cv::Mat m; frame.convertTo(m,CV_32FC3);
        cv::Mat k=(cv::Mat_<float>(3,3)<<0.131f,0.534f,0.272f,0.168f,0.686f,0.349f,0.189f,0.769f,0.393f);
        cv::transform(m,m,k); cv::threshold(m,m,255.0,255.0,cv::THRESH_TRUNC); m.convertTo(r,CV_8UC3);
    }
    else if (f=="Invert") cv::bitwise_not(frame,r);
    else if (f=="Sharpen") { cv::Mat k=(cv::Mat_<float>(3,3)<<0.f,-1.f,0.f,-1.f,5.f,-1.f,0.f,-1.f,0.f); cv::filter2D(frame,r,-1,k); }
    else r=frame.clone();
    return r;
}

void VideoProcessingWindow::setStatus(const QString& msg, StatusType type)
{
    ui->statusLabel->setText("  "+msg);
    const char* s=nullptr;
    switch(type) {
    case StatusType::Success: s="color:#48d890;background:#071510;border:1px solid #1a5c3a;border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';"; break;
    case StatusType::Error:   s="color:#ff5f5f;background:#180808;border:1px solid #5c1a1a;border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';"; break;
    default:                  s="color:#8888aa;background:#0a0a18;border:1px solid #1c1c38;border-radius:6px;padding:0 10px;font-size:12px;font-family:'Segoe UI';"; break;
    }
    ui->statusLabel->setStyleSheet(QString::fromLatin1(s));
}

// ============================================================
// Stylesheet
// ============================================================

void VideoProcessingWindow::applyStyles()
{
    setStyleSheet(R"(
QMainWindow, QWidget#centralwidget { background-color: #0d0d1c; }
QFrame#headerFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #14143a,stop:1 #1e0e38); border-bottom: 1px solid #2a2060; }
QLabel#titleLabel { color:#d0d0ff;font-family:'Segoe UI';font-size:18px;font-weight:bold;letter-spacing:3px;background:transparent; }
QLabel#subtitleLabel { color:#5a5a90;font-family:'Segoe UI';font-size:11px;background:transparent; }
QFrame#footerFrame { background-color:#08080f;border-top:1px solid #181830; }

QTabWidget#tabWidget::pane { border:none;background-color:#0d0d1c; }
QTabWidget#tabWidget > QWidget { background-color:#0d0d1c; }
QTabBar::tab { background-color:#0c0c1e;color:#5a5a90;border:1px solid #1a1a38;border-bottom:none;padding:8px 18px;font-family:'Segoe UI';font-size:12px; }
QTabBar::tab:selected { background-color:#161630;color:#c0c0ff;border-top:2px solid #5050d0; }
QTabBar::tab:hover:!selected { background-color:#12122a;color:#9090d0; }

QLabel#filterLabel,QLabel#frameLabel,QLabel#flowMethodLabel,QLabel#flowSpeedLabel,
QLabel#flowFrameLabel,QLabel#threshLabel,QLabel#timelapseModeLabel,
QLabel#timelapseFactorLabel,QLabel#timelapseFrameLabel,QLabel#stabFrameLabel {
    color:#60608a;font-family:'Segoe UI';font-size:12px;background:transparent; }
QLabel#frameCountLabel,QLabel#flowCountLabel,QLabel#threshValueLabel,
QLabel#timelapseCountLabel,QLabel#stabCountLabel {
    color:#7070a0;font-family:'Segoe UI';font-size:12px;background:transparent; }
QLabel#recPathLabel { color:#5050a0;font-family:'Segoe UI';font-size:11px;font-style:italic;background:transparent; }
QLabel#stabOrigTitleLabel,QLabel#stabResultTitleLabel {
    color:#50508a;font-family:'Segoe UI';font-size:10px;font-weight:bold;letter-spacing:1px;background:transparent; }

QLabel#filterPreviewLabel,QLabel#flowPreviewLabel,QLabel#camPreviewLabel,
QLabel#timelapsePreviewLabel,QLabel#stabOrigLabel,QLabel#stabResultLabel {
    background-color:#07070f;border:1px solid #1a1a38;border-radius:8px;color:#30304c;font-family:'Segoe UI';font-size:14px; }

QSlider::groove:horizontal { background:#141428;height:4px;border-radius:2px; }
QSlider::handle:horizontal { background:#5050c8;width:14px;height:14px;margin:-5px 0;border-radius:7px; }
QSlider::handle:horizontal:hover { background:#7070e8; }
QSlider::sub-page:horizontal { background:#3838a0;border-radius:2px; }

QProgressBar { background-color:#0a0a1c;border:1px solid #1a1a38;border-radius:4px;text-align:center;color:#8888cc;font-size:11px; }
QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3838e8,stop:1 #8820d8);border-radius:3px; }

QComboBox { background-color:#0a0a1c;border:1px solid #252550;border-radius:8px;color:#c0c0e0;padding:4px 10px;font-family:'Segoe UI';font-size:13px; }
QComboBox:hover { border-color:#4848a8;background-color:#0e0e26; }
QComboBox::drop-down { border:none;width:24px; }
QComboBox QAbstractItemView { background-color:#0d0d1c;border:1px solid #252550;color:#c0c0e0;selection-background-color:#28286a;selection-color:#ffffff; }

QPushButton { background-color:#16162e;color:#b0b0d0;border:1px solid #26264c;border-radius:8px;padding:6px 14px;font-family:'Segoe UI';font-size:12px;font-weight:500; }
QPushButton:hover { background-color:#20204a;border-color:#3c3c8a;color:#dcdcff; }
QPushButton:pressed { background-color:#0c0c20;border-color:#5858b0; }
QPushButton:disabled { background-color:#0c0c18;border-color:#141428;color:#303050; }

QPushButton#loadFilterButton,QPushButton#loadFlowButton,
QPushButton#startCamButton,QPushButton#loadTimelapseButton,QPushButton#loadStabButton {
    background-color:#0a2518;border-color:#185a35;color:#48c880; }
QPushButton#loadFilterButton:hover,QPushButton#loadFlowButton:hover,
QPushButton#startCamButton:hover,QPushButton#loadTimelapseButton:hover,QPushButton#loadStabButton:hover {
    background-color:#103020;border-color:#287a50;color:#68e8a0; }

QPushButton#stopCamButton { background-color:#250a0a;border-color:#5a1818;color:#c84848; }
QPushButton#stopCamButton:hover { background-color:#301010;border-color:#7a2828;color:#e86868; }
QPushButton#stopCamButton:disabled { background-color:#0c0c18;border-color:#141428;color:#303050; }

QPushButton#exportFilterButton,QPushButton#exportTimelapseButton {
    background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3838e8,stop:1 #8820d8);
    color:#ffffff;border:none;border-radius:8px;font-weight:bold; }
QPushButton#exportFilterButton:hover,QPushButton#exportTimelapseButton:hover {
    background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5050ff,stop:1 #a030f0); }
QPushButton#exportFilterButton:disabled,QPushButton#exportTimelapseButton:disabled { background:#141428;color:#303050; }

QPushButton#stabilizeButton { background-color:#0a1828;border-color:#183868;color:#4898d0; }
QPushButton#stabilizeButton:hover { background-color:#101e32;border-color:#285898;color:#68b8f0; }
QPushButton#stabilizeButton:disabled { background-color:#0a0a18;border-color:#141428;color:#283040; }

QPushButton#exportStabButton { background-color:#1a1428;border-color:#3a2858;color:#9060c8; }
QPushButton#exportStabButton:hover { background-color:#22183a;border-color:#5a3888;color:#b080e8; }
QPushButton#exportStabButton:disabled { background-color:#0a0a18;border-color:#141428;color:#303050; }

QPushButton#playFlowButton { background-color:#0a1828;border-color:#183868;color:#4898d0; }
QPushButton#playFlowButton:hover { background-color:#101e32;border-color:#285898;color:#68b8f0; }
QPushButton#playFlowButton:disabled { background-color:#0a0a18;border-color:#141428;color:#283040; }

QPushButton#backButton { background-color:#181424;border-color:#362850;color:#8860b8; }
QPushButton#backButton:hover { background-color:#20182e;border-color:#503878;color:#a888d8; }

QScrollBar:vertical { background:#07070f;width:6px;border-radius:3px; }
QScrollBar::handle:vertical { background:#252558;border-radius:3px;min-height:20px; }
QScrollBar::handle:vertical:hover { background:#3838a0; }
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }

QMenuBar { background-color:#08080f;color:#606090;border-bottom:1px solid #14142a; }
QMenuBar::item:selected { background-color:#1a1a38;color:#c0c0e0; }
QStatusBar { background-color:#08080f;color:#404070;font-size:11px; }
)");
}
