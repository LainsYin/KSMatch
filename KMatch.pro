#-------------------------------------------------
#
# Project created by QtCreator 2015-09-08T15:38:57
#
#-------------------------------------------------

QT       += core gui
QT       += sql
QT       += network
LIBS     += -L$$PWD/libvlc/libvlc_lib -llibvlc -llibvlccore
INCLUDEPATH +=$$PWD/libvlc/libvlc_include
DEPENDPATH  +=$$PWD/libvlc/libvlc_include
INCLUDEPATH +=$$PWD/libvlc/include
DEPENDPATH  +=$$PWD/libvlc/include
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KMatch
TEMPLATE = app

CONFIG += c++11
CONFIG += warn_off
OBJECTS_DIR += obj
UI_DIR += forms
RCC_DIR += rcc
MOC_DIR += moc
DESTDIR += bin

SOURCES += main.cpp\
        kmatch.cpp \
    tablemodel.cpp \
    senddialog.cpp \
    addsongdialog.cpp \
    mysqlquery.cpp \
    yqcdelegate.cpp \
    selectwinnerdialog.cpp \
    libvlc/src/Audio.cpp \
    libvlc/src/Common.cpp \
    libvlc/src/ControlAudio.cpp \
    libvlc/src/ControlVideo.cpp \
    libvlc/src/Enums.cpp \
    libvlc/src/Error.cpp \
    libvlc/src/Instance.cpp \
    libvlc/src/Media.cpp \
    libvlc/src/MediaList.cpp \
    libvlc/src/MediaListPlayer.cpp \
    libvlc/src/MediaPlayer.cpp \
    libvlc/src/MetaManager.cpp \
    libvlc/src/Video.cpp \
    libvlc/src/VideoDelegate.cpp \
    libvlc/src/WidgetSeek.cpp \
    libvlc/src/WidgetVideo.cpp \
    libvlc/src/WidgetVolumeSlider.cpp

HEADERS  += kmatch.h \
    tablemodel.h \
    senddialog.h \
    addsongdialog.h \
    mysqlquery.h \
    yqcdelegate.h \
    selectwinnerdialog.h \
    libvlc/include/Audio.h \
    libvlc/include/Common.h \
    libvlc/include/Config.h \
    libvlc/include/ControlAudio.h \
    libvlc/include/ControlVideo.h \
    libvlc/include/Enums.h \
    libvlc/include/Error.h \
    libvlc/include/Instance.h \
    libvlc/include/Media.h \
    libvlc/include/MediaList.h \
    libvlc/include/MediaListPlayer.h \
    libvlc/include/MediaPlayer.h \
    libvlc/include/MetaManager.h \
    libvlc/include/SharedExportCore.h \
    libvlc/include/SharedExportWidgets.h \
    libvlc/include/Video.h \
    libvlc/include/VideoDelegate.h \
    libvlc/include/WidgetSeek.h \
    libvlc/include/WidgetVideo.h \
    libvlc/include/WidgetVolumeSlider.h

FORMS    += kmatch.ui \
    senddialog.ui \
    addsongdialog.ui \
    selectwinnerdialog.ui

RESOURCES += \
    resources.qrc

DISTFILES +=
