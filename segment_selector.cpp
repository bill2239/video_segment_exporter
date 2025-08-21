#include "segment_selector.h"
#include <algorithm>

SegmentSelectorWidget::SegmentSelectorWidget(int totalFrames, QWidget* parent)
    : QWidget(parent), total_frames(totalFrames), start_frame(0), end_frame(totalFrames-1), dragging("") {
    setFixedHeight(40);
}

void SegmentSelectorWidget::setRange(int totalFrames) {
    total_frames = totalFrames;
    end_frame = totalFrames - 1;
    update();
}

void SegmentSelectorWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    int w = width(), h = height();

    painter.setBrush(QColor(200,200,200));
    painter.drawRect(0,h/3,w,h/3);

    int x1 = frameToPixel(start_frame);
    int x2 = frameToPixel(end_frame);

    painter.setBrush(QColor(100,150,255));
    painter.drawRect(x1,h/3,x2-x1,h/3);

    painter.setBrush(QColor(255,50,50));
    painter.drawRect(x1-2,0,4,h);
    painter.drawRect(x2-2,0,4,h);
}

void SegmentSelectorWidget::mousePressEvent(QMouseEvent* event) {
    int pos = pixelToFrame(event->x());
    if(abs(pos-start_frame) < total_frames/100.0) dragging="start";
    else if(abs(pos-end_frame) < total_frames/100.0) dragging="end";
}

void SegmentSelectorWidget::mouseMoveEvent(QMouseEvent* event) {
    if(!dragging.isEmpty()) {
        int frame = std::clamp(pixelToFrame(event->x()),0,total_frames-1);
        if(dragging=="start") start_frame = std::min(frame,end_frame);
        else if(dragging=="end") end_frame = std::max(frame,start_frame);
        emit segmentChanged(start_frame,end_frame);
        update();
    }
}

void SegmentSelectorWidget::mouseReleaseEvent(QMouseEvent*) {
    dragging="";
}

int SegmentSelectorWidget::frameToPixel(int frame) {
    return int(frame*width()/total_frames);
}

int SegmentSelectorWidget::pixelToFrame(int pixel) {
    return int(pixel*total_frames/width());
}
