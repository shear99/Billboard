QT += core gui widgets multimedia multimediawidgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += include

SOURCES += \
    src/infobox.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/subtitle.cpp \
    src/videoplayer.cpp

HEADERS += \
    include/infobox.h \
    include/mainwindow.h \
    include/subtitle.h \
    include/videoplayer.h

FORMS += \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 라즈베리파이에서 하드웨어 가속 사용
CONFIG += link_pkgconfig
unix: PKGCONFIG += gstreamer-1.0