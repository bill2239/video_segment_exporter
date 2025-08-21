#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QTimer>
#include <QString>
#include "segment_selector.h"
#include <opencv2/opencv.hpp>
#include "gif.h"

class VideoPlayer : public QWidget {
    Q_OBJECT
public:
    VideoPlayer(int width=640, int height=480, int fps=30, QWidget* parent=nullptr);

private slots:
    void loadVideo();
    void displayVideo();
    void playPause();
    void exportFrames();
    void exportSegmentVideo();
    void exportGIF(double duration=0.1);
    void updateSegment(int start, int end);

private:
    QLabel* frame_label;
    QPushButton* play_button;
    QPushButton* load_button;
    QPushButton* export_button;
    QPushButton* export_video_button;
    QPushButton* export_gif_button;
    SegmentSelectorWidget* selector;
    QTimer* timer;

    cv::VideoCapture cap;
    std::string video_path;
    int total_frames = 0;
    int start_frame = 0;
    int end_frame = 0;
    QSize video_size;
    int fps;
};
