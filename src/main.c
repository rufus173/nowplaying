#include "media_players.h"
#include <stdio.h>
#include <assert.h>
#include <glib.h>
int main(){
	players_t *players_list = get_players_list(0);
	assert(players_list);

	media_t *current_media = get_currently_playing_media(players_list,0);
	free_currently_playing_media(current_media);

	//GMainLoop *mainloop = g_main_loop_new(NULL,FALSE);
	//g_main_loop_run(mainloop);
	

	free_players_list(players_list);
}
