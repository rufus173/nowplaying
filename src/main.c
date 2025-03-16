#include "media_players.h"
#include <stdio.h>
int main(){
	players_t *players_list = get_players_list();
	free_players_list(players_list);
}
