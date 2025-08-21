#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class SegmentSelectorWidget : public QWidget {
    Q_OBJECT

public:
    SegmentSelectorWidget(int totalFrames = 100, QWidget* parent = nullptr);

    void setRange(int totalFrames);

signals:
    void segmentChanged(int start, int end);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    int frameToPixel(int frame);
    int pixelToFrame(int pixel);

    int total_frames;
    int start_frame;
    int end_frame;
    QString dragging;
};
