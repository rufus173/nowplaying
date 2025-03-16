#include "media_players.h"
#include <stdio.h>
#include <assert.h>
#include <glib.h>
int main(){
	players_t *players_list = get_players_list();
	assert(players_list);
	GMainLoop *mainloop = g_main_loop_new(NULL,FALSE);
	g_main_loop_run(mainloop);
	free_players_list(players_list);
}
