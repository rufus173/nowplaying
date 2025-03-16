CFLAGS=-g -Wall `pkg-config --cflags glib-2.0 gio-2.0`
LDFLAGS=`pkg-config --libs glib-2.0 gio-2.0` -fsanitize=address

OBJ_FILES=src/media_players.o src/main.o

nowplaying : $(OBJ_FILES)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
clean :
	rm $(OBJ_FILES)
