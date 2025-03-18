CC = g++
CFLAGS = -g -Wall `pkg-config --cflags glib-2.0 gio-2.0`
LDFLAGS = -fsanitize=address `pkg-config --libs glib-2.0 gio-2.0`
LDFLAGS+ = -lQtCore -lQtGui
EXECUTABLE = nowplaying

SOURCES = src/media_players/media_players.c src/qt_interface/main.cpp

OBJECTS = $(addsuffix .o,$(basename $(SOURCES)))

.PHONY : nowplaying

$(EXECUTABLE) : $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
