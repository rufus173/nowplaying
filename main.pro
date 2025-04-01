TEMPLATE = app
HEADERS += src/media_players.h src/main.h
SOURCES += src/main.cpp src/media_players.cpp
TARGET = nowplaying
QT = core gui dbus
QT += widgets
CONFIG += debug

to_install.files = nowplaying
to_install.path = /usr/bin
INSTALLS += to_install
