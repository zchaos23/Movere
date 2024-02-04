QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    toolbar.cpp \
    vlccontrol.cpp

HEADERS += \
    mainwindow.h \
    toolbar.h \
    vlccontrol.h

FORMS += \
    mainwindow.ui \
    toolbar.ui

INCLUDEPATH += $$PWD/libvlc/sdk/include
DEPENDPATH += $$PWD/libvlc/sdk/lib
# LIBS += $$PWD/libvlc/sdk/lib -llibvlc.lib -llibvlccore.lib
LIBS += $$PWD/libvlc/sdk/lib/libvlc.lib
LIBS += $$PWD/libvlc/sdk/lib/libvlccore.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icon/Movere_icon.ico \
    icon/close.svg \
    icon/fullscreen-exit.svg \
    icon/myIcon.rc \
    icon/new-folder.svg \
    icon/pause.svg \
    icon/play.svg \
    icon/volume-high.svg \
    icon/volume-off.svg

RC_FILE += \
    icon/myIcon.rc

TARGET = Movere
