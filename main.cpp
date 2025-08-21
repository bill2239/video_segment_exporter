#include <QApplication>
#include "video_player.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    VideoPlayer player;
    player.setWindowTitle("Video Segment Selector");
    player.resize(700,600);
    player.show();
    return app.exec();
}

