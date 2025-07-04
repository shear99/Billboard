#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 라즈베리파이에서 최적화를 위한 설정
    #ifdef Q_OS_LINUX
        // 하드웨어 가속 활성화
        qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");
        // V4L2 백엔드 사용 (라즈베리파이 최적화)
        qputenv("QT_GSTREAMER_USE_PLAYBIN_VOLUME", "true");
    #endif

    MainWindow window;
    window.showFullScreen();

    return app.exec();
}