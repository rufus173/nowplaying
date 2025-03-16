#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <sys/queue.h>
#include "media_players.h"
#include <pthread.h>

//logging and errors
#define IF_VERBOSE if(1)
#define IF_PRINT_ERRORS if(1)

//====== players ======
/*
returns linked list of players that then must be freed by free_players_list
basicaly runs `dbus-send --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.ListNames | grep "org.mpris"` and stores the output
*/
players_t *get_players_list(){
	//====== allocate things for later ======
	players_t *players = malloc(sizeof(players_t));
	memset(players,0,sizeof(players_t));
	GError *bus_error = NULL;

	//====== setup players ======
	pthread_mutex_init(&players->mutex,NULL); //cant fail
	pthread_mutex_lock(&players->mutex);

	//====== create a proxy object ======
	IF_VERBOSE printf("%s: creating proxy\n",__FUNCTION__);
	//connects for us and creates a proxy object
	GDBusProxy *bus_info_proxy; //                       session bus    flags (not important)       connect to main bus         object path         interface name
	bus_info_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,G_DBUS_PROXY_FLAGS_NONE,NULL,"org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus",NULL,&bus_error);
	if (bus_info_proxy == NULL){
		IF_PRINT_ERRORS fprintf(stderr,"%s\n",bus_error->message);
		pthread_mutex_unlock(&players->mutex);
		free_players_list(players);
		return NULL;
	}
	IF_VERBOSE printf("%s: proxy created\n",__FUNCTION__ );
	players->bus_info_proxy = bus_info_proxy;

	//====== call ListNames on bus proxy (get all things on the bus) ======
	GVariant *names;
	names = g_dbus_proxy_call_sync(bus_info_proxy,"ListNames",NULL,G_DBUS_CALL_FLAGS_NONE,-1,NULL,&bus_error);
	if (names == NULL){
		IF_PRINT_ERRORS fprintf(stderr,"%s\n",bus_error->message);
		pthread_mutex_unlock(&players->mutex);
		free_players_list(players);
		return NULL;
	}

	//====== cleanup and return ======
	g_variant_unref(names);
	pthread_mutex_unlock(&players->mutex);
	return players;
}
/*
frees up the linked list of players
*/
void free_players_list(players_t *players){
	pthread_mutex_destroy(&players->mutex);
	if (players->bus_info_proxy != NULL){
		g_object_unref(players->bus_info_proxy);
	}
	free(players);
}
int update_players_list(players_t *players){
}

//====== media ======
media_t *get_currently_playing_media(players_t *players,int flags){
}
void free_currently_playing_media(media_t *media){
}
