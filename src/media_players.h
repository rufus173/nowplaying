#ifndef _MEDIA_PLAYERS_H
#define _MEDIA_PLAYERS_H

#include <sys/queue.h>
#include <pthread.h>
#include <gio/gio.h>

//====== structs and types ======
struct players_entry { //linked list of players
	char *address;
	LIST_ENTRY(players_entry) next;
};
LIST_HEAD(players_head, players_entry);
struct media {
};
struct players { //holds info about players as well as linked list of players
	pthread_mutex_t mutex;
	GDBusProxy *bus_info_proxy;
	struct players_head *players_list;
};
typedef struct players players_t;
typedef struct media media_t;

//====== players ======
players_t *get_players_list();
void free_players_list(players_t *players);
int update_players_list(players_t *players);

//====== media ======
media_t *get_currently_playing_media(players_t *players,int flags);
void free_currently_playing_media(media_t *media);

#endif
