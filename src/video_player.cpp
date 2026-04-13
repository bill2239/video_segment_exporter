#include "video_player.h"
#include "segment_selector.h"
#include "gif.h"

#include <QGridLayout>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QCollator>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QtConcurrent/QtConcurrent>
#include <opencv2/opencv.hpp>

VideoPlayer::VideoPlayer(int width, int height, int fps, QWidget* parent)
    : QWidget(parent), video_size(width, height), fps(fps)
{
    // UI Elements
    frame_label = new QLabel(this);
    selector = new SegmentSelectorWidget();
    play_button = new QPushButton("Pause", this);
    load_button = new QPushButton("Load Video / Directory", this);
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
    QString path = QFileDialog::getOpenFileName(
        this,
        "Open Video",
        QString(),
        "Video Files (*.mp4 *.mov *.avi *.mkv *.webm *.m4v);;All Files (*)");

    if (path.isEmpty()) {
        path = QFileDialog::getExistingDirectory(this, "Open Image Directory");
    }

    if (path.isEmpty()) return;

    openMediaSource(path);
}

void VideoPlayer::displayVideo() {
    if (total_frames <= 0) return;
    if (current_frame > end_frame) current_frame = start_frame;

    cv::Mat frame = readFrameAt(current_frame);
    if (frame.empty()) return;
    current_frame++;

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    frame_label->setPixmap(QPixmap::fromImage(img).scaled(video_size.width(), video_size.height()));
}

void VideoPlayer::playPause() {
    if (timer->isActive()) timer->stop();
    else timer->start(1000 / fps);
}

void VideoPlayer::exportFrames() {
    if (total_frames <= 0) return;
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (dir.isEmpty()) return;

    for (int frame_num = start_frame; frame_num <= end_frame; ++frame_num) {
        cv::Mat frame = readFrameAt(frame_num);
        if (frame.empty()) break;
        std::string filename = dir.toStdString() + "/frame_" + std::to_string(frame_num) + ".jpg";
        cv::imwrite(filename, frame);
    }
}

void VideoPlayer::exportSegmentVideo() {
    if (total_frames <= 0) return;
    QString out_path = QFileDialog::getSaveFileName(this, "Save Video Segment", "segment.mp4", "Videos (*.mp4)");
    if (out_path.isEmpty()) return;

    cv::Mat firstFrame = readFrameAt(start_frame);
    if (firstFrame.empty()) return;

    int width = firstFrame.cols;
    int height = firstFrame.rows;
    double fps_ = loaded_from_directory ? fps : cap.get(cv::CAP_PROP_FPS);
    if (fps_ <= 0) fps_ = fps;

    cv::VideoWriter writer(out_path.toStdString(),
                           cv::VideoWriter::fourcc('m','p','4','v'),
                           fps_,
                           cv::Size(width, height));

    for (int frame_num = start_frame; frame_num <= end_frame; ++frame_num) {
        cv::Mat frame = readFrameAt(frame_num);
        if (frame.empty()) break;
        writer.write(frame);
    }

    writer.release();
}


void VideoPlayer::exportGIF(double duration) {
    if (total_frames <= 0) return;
    QString out_gif = QFileDialog::getSaveFileName(this, "Save GIF", "output.gif", "GIF (*.gif)");
    if (out_gif.isEmpty()) return;

    std::string videoPath = video_path;
    QStringList imagePaths = image_paths;
    bool fromDirectory = loaded_from_directory;
    int start = start_frame;
    int end = end_frame;

    [[maybe_unused]] auto gifExport = QtConcurrent::run([=]() {
        auto loadFrame = [&](int frameIndex) -> cv::Mat {
            if (fromDirectory) {
                if (frameIndex < 0 || frameIndex >= imagePaths.size()) return cv::Mat();
                return cv::imread(imagePaths.at(frameIndex).toStdString(), cv::IMREAD_COLOR);
            }

            cv::VideoCapture localCap(videoPath);
            if (!localCap.isOpened()) return cv::Mat();
            localCap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);
            cv::Mat localFrame;
            if (!localCap.read(localFrame)) return cv::Mat();
            return localFrame;
        };

        cv::Mat frame = loadFrame(start);
        if (frame.empty()) return;

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);

        GifWriter g;
        GifBegin(&g, out_gif.toStdString().c_str(), frame.cols, frame.rows, int(duration * 100));
        GifWriteFrame(&g, frame.data, frame.cols, frame.rows, int(duration * 100));

        for (int frameNum = start + 1; frameNum <= end; ++frameNum) {
            frame = loadFrame(frameNum);
            if (frame.empty()) break;
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
            GifWriteFrame(&g, frame.data, frame.cols, frame.rows, int(duration * 100));
        }

        GifEnd(&g);
    });
}

void VideoPlayer::updateSegment(int start, int end) {
    start_frame = start;
    end_frame = end;
    current_frame = start_frame;
}

bool VideoPlayer::openMediaSource(const QString& path) {
    cap.release();
    image_paths.clear();
    loaded_from_directory = false;
    video_path.clear();
    total_frames = 0;

    QFileInfo info(path);
    if (info.isDir()) {
        QDir dir(path);
        const QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tif", "*.tiff", "*.webp"};
        image_paths = dir.entryList(filters, QDir::Files, QDir::Name);
        for (QString& fileName : image_paths) {
            fileName = dir.absoluteFilePath(fileName);
        }

        QCollator collator;
        collator.setNumericMode(true);
        std::sort(image_paths.begin(), image_paths.end(),
                  [&](const QString& left, const QString& right) {
                      return collator.compare(left, right) < 0;
                  });

        if (image_paths.isEmpty()) return false;

        loaded_from_directory = true;
        total_frames = image_paths.size();
    } else {
        video_path = path.toStdString();
        cap.open(video_path);
        if (!cap.isOpened()) return false;
        total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        if (total_frames <= 0) return false;
    }

    selector->setRange(total_frames);
    start_frame = 0;
    end_frame = total_frames - 1;
    current_frame = start_frame;
    return true;
}

cv::Mat VideoPlayer::readFrameAt(int frameIndex) {
    if (frameIndex < 0 || frameIndex >= total_frames) return cv::Mat();

    if (loaded_from_directory) {
        return cv::imread(image_paths.at(frameIndex).toStdString(), cv::IMREAD_COLOR);
    }

    if (!cap.isOpened()) return cv::Mat();
    cap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);
    cv::Mat frame;
    if (!cap.read(frame)) return cv::Mat();
    return frame;
}
