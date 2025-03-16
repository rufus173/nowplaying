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
#include <assert.h>

//logging and errors
#define IF_VERBOSE if(1)
#define IF_PRINT_ERRORS if(1)

static void _bus_add_remove_callback(GDBusProxy *dbus_proxy,gchar *sender_name,gchar *signal_name,GVariant *parameters,gpointer user_data){
	players_t *players = user_data;
	
	//====== filter for mpris related ones ======
	GVariant *arg_0 = g_variant_get_child_value(parameters,0);
	GVariant *arg_1 = g_variant_get_child_value(parameters,1);
	GVariant *arg_2 = g_variant_get_child_value(parameters,2);
	const gchar *name = g_variant_get_string(arg_0,NULL);
	if (strstr(name,"org.mpris.MediaPlayer2") == name){ //org.mpr... at start of string (hence equal to original string pointer)
		/* debug info 
		gchar *variant_pretty = g_variant_print(parameters,FALSE);
		IF_VERBOSE printf("%s: sender %s send signal %s\n",__FUNCTION__,sender_name,signal_name);
		IF_VERBOSE printf("%s: %s\n",__FUNCTION__,variant_pretty);
		g_free(variant_pretty);
		*/
		
		//====== aquire mutex ======
		assert(pthread_mutex_lock(&players->mutex) == 0);

		//====== further filter for added and removed ======
		if (strcmp(g_variant_get_string(arg_1,NULL),"") == 0){
			//====== added ======
			IF_VERBOSE printf("%s: %s added\n",__FUNCTION__,name);

			//allocate entry
			struct players_entry *new_entry = malloc(sizeof(struct players_entry));
			memset(new_entry,0,sizeof(struct players_entry));
			
			//strdup as we dont own the data right now
			new_entry->address = strdup(name);

			//add it to list
			LIST_INSERT_HEAD(players->players_list,new_entry,next);
		}
		else if (strcmp(g_variant_get_string(arg_2,NULL),"") == 0){
			//====== removed ======
			//find relevant entry
			for (struct players_entry *current = LIST_FIRST(players->players_list);current != NULL;){
				struct players_entry *next = LIST_NEXT(current,next);
				
				//check if the name matches
				if (strcmp(name,current->address)){
					IF_VERBOSE printf("%s: %s removed\n",__FUNCTION__,name);
					LIST_REMOVE(current,next);
					free(current->address);
					free(current);
				}

				current = next;
			}
		}
		
		//====== release mutex ======
		assert(pthread_mutex_unlock(&players->mutex) == 0);
	}
	g_variant_unref(arg_0);

}

//====== players ======
/*
returns linked list of players that then must be freed by free_players_list
basicaly runs `dbus-send --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.ListNames | grep "org.mpris"` and stores the output
*/
players_t *get_players_list(int flags){
	//====== allocate things for later ======
	players_t *players = malloc(sizeof(players_t));
	memset(players,0,sizeof(players_t));
	GError *bus_error = NULL;

	//====== setup players ======
	pthread_mutex_init(&players->mutex,NULL); //cant fail
	pthread_mutex_lock(&players->mutex);
	players->players_list = malloc(sizeof(struct players_head));
	LIST_INIT(players->players_list);

	//====== create a proxy object ======
	IF_VERBOSE printf("%s: creating proxy\n",__FUNCTION__);
	//connects for us and creates a proxy object
	GDBusProxy *bus_info_proxy; //                       session bus    flags (not important)       connect to main bus         object path         interface name
	bus_info_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,G_DBUS_PROXY_FLAGS_NONE,NULL,"org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus",NULL,&bus_error);
	if (bus_info_proxy == NULL){
		IF_PRINT_ERRORS fprintf(stderr,"%s\n",bus_error->message);
		pthread_mutex_unlock(&players->mutex);
		free_players_list(players);
		g_error_free(bus_error);
		return NULL;
	}
	IF_VERBOSE printf("%s: proxy created\n",__FUNCTION__ );
	players->bus_info_proxy = bus_info_proxy;

	//====== call ListNames on bus proxy (get all things on the bus) ======
	GVariant *dbus_result;
	dbus_result = g_dbus_proxy_call_sync(bus_info_proxy,"ListNames",NULL,G_DBUS_CALL_FLAGS_NONE,-1,NULL,&bus_error);
	if (dbus_result == NULL){
		IF_PRINT_ERRORS fprintf(stderr,"%s\n",bus_error->message);
		pthread_mutex_unlock(&players->mutex);
		free_players_list(players);
		g_error_free(bus_error);
		return NULL;
	}
	
	//====== connect signal to detect when players added / removed ======
	g_signal_connect(bus_info_proxy,"g-signal::NameOwnerChanged",G_CALLBACK(_bus_add_remove_callback),players);

	//====== itterate over returned list to find mpris addresses ======
	IF_VERBOSE printf("%s: searching for mpris addresses...\n",__FUNCTION__);
	//extract index 1 of argv tuple
	GVariant *addresses = g_variant_get_child_value(dbus_result,0);
	//create iterator
	GVariantIter *address_iterator;
	address_iterator = g_variant_iter_new(addresses);
	//iterate
	for (GVariant *next = g_variant_iter_next_value(address_iterator); next != NULL; next = g_variant_iter_next_value(address_iterator)){
		//extract string value
		gsize address_length;
		const gchar *address_string = g_variant_get_string(next,&address_length);

		//check if it begins with org.mpris.MediaPlayer2
		if (strstr(address_string,"org.mpris.MediaPlayer2") == address_string){ //ensures it is at the start of the string
			IF_VERBOSE printf("%s: found %s\n",__FUNCTION__,address_string);
			//====== add to linked list of players ======
			//allocate entry
			struct players_entry *new_entry = malloc(sizeof(struct players_entry));
			memset(new_entry,0,sizeof(struct players_entry));
			
			//strdup as we dont own the data right now
			new_entry->address = strdup(address_string);

			//add it to list
			LIST_INSERT_HEAD(players->players_list,new_entry,next);
		}

		//cleanup
		g_variant_unref(next);
	}
	g_variant_iter_free(address_iterator);
	IF_VERBOSE printf("%s: search complete\n",__FUNCTION__);

	//====== cleanup and return ======
	g_variant_unref(addresses);
	g_variant_unref(dbus_result);
	pthread_mutex_unlock(&players->mutex);
	return players;
}
/*
frees up the linked list of players
*/
void free_players_list(players_t *players){
	pthread_mutex_destroy(&players->mutex);
	if (players->bus_info_proxy != NULL){
		g_signal_handlers_disconnect_by_func(players->bus_info_proxy,G_CALLBACK(_bus_add_remove_callback),players);
		g_object_unref(players->bus_info_proxy);
	}
	
	//free everything related to the linked list
	for (struct players_entry *current = LIST_FIRST(players->players_list);current != NULL;){
		struct players_entry *next = LIST_NEXT(current,next);
		
		free(current->address);
		free(current);

		current = next;
	}

	free(players->players_list);
	free(players);
}
int update_players_list(players_t *players){
	return -1;
}

//====== media ======
/*
check each player to see if it is playing
dbus-send --print-reply --dest=org.mpris.MediaPlayer2.vlc /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get string:org.mpris.MediaPlayer2.Player string:PlaybackStatus
if it is get its metadata
dbus-send --print-reply --dest=org.mpris.MediaPlayer2.vlc /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get string:org.mpris.MediaPlayer2.Player string:Metadata
*/
media_t *get_currently_playing_media(players_t *players,int flags){
	//====== prep struct to return to user ======
	media_t *current_media = malloc(sizeof(media_t));
	//aquire mutex
	assert(pthread_mutex_lock(&players->mutex) == 0);
	
	//====== iterate over all players and find the first one playing something ======
	for (struct players_entry *current = LIST_FIRST(players->players_list);current != NULL;current = LIST_NEXT(current,next)){
		IF_VERBOSE printf("%s: checking player %s\n",__FUNCTION__,current->address);
		GError *bus_error = NULL;
		GDBusProxy *player_proxy;
		
		//====== create proxy object ======
		player_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,G_DBUS_PROXY_FLAGS_NONE,NULL,current->address,"/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties",NULL,&bus_error);
		if (player_proxy == NULL){
			IF_PRINT_ERRORS fprintf(stderr,"%s\n",bus_error->message);
			pthread_mutex_unlock(&players->mutex);
			free(current_media);
			g_error_free(bus_error);
			return NULL;
		}

		//====== get playback status  ======
		GVariant *dbus_result;
		GVariant *parameters = g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "PlaybackStatus");
		dbus_result = g_dbus_proxy_call_sync(player_proxy,"Get",parameters,G_DBUS_CALL_FLAGS_NONE,-1,NULL,&bus_error);
		if (dbus_result == NULL){
			IF_PRINT_ERRORS fprintf(stderr,"%s\n",bus_error->message);
			pthread_mutex_unlock(&players->mutex);
			free(current_media);
			g_error_free(bus_error);
			return NULL;
		}
		//there is like a nested variant for some reason
		GVariant *arg_0 = g_variant_get_child_value(dbus_result,0);
		GVariant *arg_0_variant = g_variant_get_child_value(arg_0,0);
		const gchar *playback_status = g_variant_get_string(arg_0_variant,NULL);
		IF_VERBOSE printf("%s: playback status - %s\n",__FUNCTION__,playback_status);

		//====== store the player's media data only if it is playing or paused ======
		if (strcmp(playback_status,"Playing") == 0 || strcmp(playback_status,"Paused") == 0){

			//this isnt particularly readable but im gonna use the fact that the second char in "playing", "paused", and "stopped" is unique to determine which one is is using a case statement.
			switch(playback_status[1]){
				//  p l aying
				//    ^
				case 'l':
					current_media->playback_status = MEDIA_PLAYING;
					break;
				//  p a used
				//    ^
				case 'a':
					current_media->playback_status = MEDIA_PAUSED;
					break;
				//  s t opped
				//    ^
				case 't':
					current_media->playback_status = MEDIA_STOPPED;
					break;
			}

			IF_VERBOSE printf("%s: getting metadata...\n",__FUNCTION__);

			//====== break because we have our media and we are only returning one struct ======
			current = NULL; //stopping condition for for loop so we finnish cleanup
		}


		//====== cleanup ======
		g_object_unref(player_proxy);
		g_variant_unref(arg_0);
		g_variant_unref(arg_0_variant);
		g_variant_unref(dbus_result);
	}

	//====== return and cleanup ======
	//release mutex
	assert(pthread_mutex_unlock(&players->mutex) == 0);
	return current_media;
}
void free_currently_playing_media(media_t *media){
	free(media);
}
