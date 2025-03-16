#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <sys/queue.h>
#include "media_players.h"

//logging and errors
#define IF_VERBOSE if(1)
#define IF_PRINT_ERRORS if(1)

static char *_get_session_bus_address(){
	char *address;
	address = getenv("DBUS_SESSION_BUS_ADDRESS");
	if (address == NULL){
		IF_PRINT_ERRORS fprintf(stderr,"DBUS_SESSION_BUS_ADDRESS unset; cannot asertain session bus address.\n");
		return "";
	}
	return address;
}

//====== players ======
/*
returns linked list of players that then must be freed by free_players_list
basicaly runs `dbus-send --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.ListNames | grep "org.mpris"` and stores the output
*/
players_t *get_players_list(){
	//====== allocate things for later ======
	players_t *players_list = malloc(sizeof(players_t));
	DBusError dbus_error;

	//====== connect to dbus ======
	DBusConnection *dbus_connection = dbus_connection_open(_get_session_bus_address(),&dbus_error);
	if (dbus_connection == NULL){
		IF_PRINT_ERRORS fprintf(stderr,"%s: %s\n",dbus_error.name,dbus_error.message);
		free(players_list);
		dbus_error_free(&dbus_error);
		return NULL;
	}
	
	//====== cleanup and return ======
	dbus_connection_unref(dbus_connection);
	return players_list;
}
/*
frees up the linked list of players
*/
void free_players_list(players_t *players){
	   free(players);
}
int update_players_list(players_t *players){
}

//====== media ======
media_t *get_currently_playing_media(players_t *players,int flags){
}
void free_currently_playing_media(media_t *media){
}
