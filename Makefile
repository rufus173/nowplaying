CFLAGS=-g -Wall `pkg-config --cflags dbus-1`
LDFLAGS=-fsanitize=address `pkg-config --libs dbus-1`

OBJ_FILES=src/media_players.o src/main.o

nowplaying : $(OBJ_FILES)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean :
	rm $(OBJ_FILES)
