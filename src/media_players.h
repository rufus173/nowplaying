#ifndef _MEDIA_PLAYERS_H
#define _MEDIA_PLAYERS_H

#include <sys/queue.h>

//====== structs and types ======
struct players { //linked list of players
	char *dest;
	LIST_ENTRY(players) next;
};
struct media {
};

LIST_HEAD(players_head, players);
typedef struct players_head players_t;
typedef struct media media_t;

//====== players ======
players_t *get_players_list();
void free_players_list(players_t *players);
int update_players_list(players_t *players);

//====== media ======
media_t *get_currently_playing_media(players_t *players,int flags);
void free_currently_playing_media(media_t *media);

#endif
