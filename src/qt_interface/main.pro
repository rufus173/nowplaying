TEMPLATE = app
HEADERS += media_players.h main.h
SOURCES += main.cpp media_players.cpp
TARGET = nowplaying
QT = core gui dbus
QT += widgets
CONFIG += debug
