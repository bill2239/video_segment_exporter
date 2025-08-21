#include "video_player.h"
#include "segment_selector.h"
#include "gif.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <opencv2/opencv.hpp>

VideoPlayer::VideoPlayer(int width, int height, int fps, QWidget* parent)
    : QWidget(parent), video_size(width, height), fps(fps)
{
    // UI Elements
    frame_label = new QLabel(this);
    selector = new SegmentSelectorWidget();
    play_button = new QPushButton("Pause", this);
    load_button = new QPushButton("Load Video", this);
    export_button = new QPushButton("Export Frames", this);
    export_video_button = new QPushButton("Export Video Segment", this);
    export_gif_button = new QPushButton("Frames → GIF", this);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(frame_label, 0, 0, 1, 3);
    layout->addWidget(selector, 1, 0, 1, 3);
    layout->addWidget(load_button, 2, 0);
    layout->addWidget(play_button, 2, 1);
    layout->addWidget(export_button, 2, 2);
    layout->addWidget(export_video_button, 3, 0, 1, 2);
    layout->addWidget(export_gif_button, 3, 2);
    setLayout(layout);

    // Timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VideoPlayer::displayVideo);
    connect(load_button, &QPushButton::clicked, this, &VideoPlayer::loadVideo);
    connect(play_button, &QPushButton::clicked, this, &VideoPlayer::playPause);
    connect(export_button, &QPushButton::clicked, this, &VideoPlayer::exportFrames);
    connect(export_video_button, &QPushButton::clicked, this, &VideoPlayer::exportSegmentVideo);
    connect(export_gif_button, &QPushButton::clicked, this, &VideoPlayer::exportGIF);
    connect(selector, &SegmentSelectorWidget::segmentChanged, this, &VideoPlayer::updateSegment);

    timer->start(1000 / fps);
}

// -------- Slots --------

void VideoPlayer::loadVideo() {
    QString path = QFileDialog::getOpenFileName(this, "Open Video");
    if (path.isEmpty()) return;

    video_path = path.toStdString();
    cap.open(video_path);
    if (!cap.isOpened()) return;

    total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    selector->setRange(total_frames);
    start_frame = 0;
    end_frame = total_frames - 1;
}

void VideoPlayer::displayVideo() {
    if (!cap.isOpened()) return;

    int pos = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
    if (pos > end_frame) cap.set(cv::CAP_PROP_POS_FRAMES, start_frame);

    cv::Mat frame;
    if (!cap.read(frame)) return;

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    frame_label->setPixmap(QPixmap::fromImage(img).scaled(video_size.width(), video_size.height()));
}

void VideoPlayer::playPause() {
    if (timer->isActive()) timer->stop();
    else timer->start(1000 / fps);
}

void VideoPlayer::exportFrames() {
    if (!cap.isOpened()) return;
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (dir.isEmpty()) return;

    cap.set(cv::CAP_PROP_POS_FRAMES, start_frame);
    int frame_num = start_frame;

    while (frame_num <= end_frame) {
        cv::Mat frame;
        if (!cap.read(frame)) break;
        std::string filename = dir.toStdString() + "/frame_" + std::to_string(frame_num) + ".jpg";
        cv::imwrite(filename, frame);
        frame_num++;
    }
}

void VideoPlayer::exportSegmentVideo() {
    if (!cap.isOpened()) return;
    QString out_path = QFileDialog::getSaveFileName(this, "Save Video Segment", "segment.mp4", "Videos (*.mp4)");
    if (out_path.isEmpty()) return;

    cap.set(cv::CAP_PROP_POS_FRAMES, start_frame);
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps_ = cap.get(cv::CAP_PROP_FPS);

    cv::VideoWriter writer(out_path.toStdString(),
                           cv::VideoWriter::fourcc('m','p','4','v'),
                           fps_,
                           cv::Size(width, height));

    int frame_num = start_frame;
    while (frame_num <= end_frame) {
        cv::Mat frame;
        if (!cap.read(frame)) break;
        writer.write(frame);
        frame_num++;
    }

    writer.release();
}

void VideoPlayer::exportGIF(double duration) {
    if (!cap.isOpened()) return;
    QString out_gif = QFileDialog::getSaveFileName(this, "Save GIF", "output.gif", "GIF (*.gif)");
    if (out_gif.isEmpty()) return;

    cap.set(cv::CAP_PROP_POS_FRAMES, start_frame);
    int frame_num = start_frame;

    cv::Mat frame;
    if (!cap.read(frame)) return;

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);

    GifWriter g;
    GifBegin(&g, out_gif.toStdString().c_str(), frame.cols, frame.rows, int(duration * 100),8,true);
    GifWriteFrame(&g, frame.data, frame.cols, frame.rows, int(duration * 100),8,true);
    frame_num++;

    while (frame_num <= end_frame) {
        if (!cap.read(frame)) break;
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
        GifWriteFrame(&g, frame.data, frame.cols, frame.rows, int(duration * 100));
        frame_num++;
    }

    GifEnd(&g);
}

void VideoPlayer::updateSegment(int start, int end) {
    start_frame = start;
    end_frame = end;
}
