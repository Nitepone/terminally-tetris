#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "generic.h"
#include "list.h"
#include "log.h"
#include "player.h"
#include "tetris_game.h"

static struct st_list *player_list;

void player_init() { player_list = list_create(); }

void *player_clock(void *input) {
	struct st_player *player = (struct st_player *)input;
	fprintf(logging_fp, "player_clock: thread started\n");
	do {
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
		lower_block(1, player->contents);

		// send board to player and clear socket if a failure occurs
		if (player->render(player->fd, player) == EXIT_FAILURE)
			player->fd = -1;

		// send board to opponent if one exists
		if (player->opponent)
			player->render(player->opponent->fd, player);

	} while (game_over(player->contents) == 0);
	fprintf(logging_fp, "player_clock: thread exiting\n");
	return 0;
}

void start_game(struct st_player *player) {
	pthread_create(&player->game_clk_thread, NULL, player_clock,
	               (void *)player);
}

struct st_player *get_player_from_fd(int fd) {
	struct st_player *player;
	for (int i = 0; i < player_list->length; i++) {
		player = (struct st_player *)(list_get(player_list, i)->target);
		if (player->fd == fd)
			return player;
	}
	return 0;
}

StringArray *player_names() {
	StringArray *arr = string_array_create(player_list->length);

	for (int i = 0; i < player_list->length; i++) {
		Player *player =
		    (struct st_player *)(list_get(player_list, i)->target);
		string_array_set_item(arr, i, player->name);
	}

	return arr;
}

/**
 * Get a player by name
 *
 * WARNING: this is a comparitively expensive operation.
 */
Player *player_get_by_name(char *name) {
	struct st_player *player;
	for (int i = 0; i < player_list->length; i++) {
		player = (struct st_player *)(list_get(player_list, i)->target);
		fprintf(logging_fp,
		        "player_get_by_name: Checking player '%s'\n",
		        player->name);
		if (strcmp(player->name, name) == 0)
			return player;
	}
	fprintf(logging_fp, "player_get_by_name: No player found for '%s'\n",
	        name);
	return NULL;
}

struct st_player *player_create(int fd, char *name) {
	struct st_player *player = malloc(sizeof(struct st_player));
	player->fd = fd;
	player->name = malloc(strlen(name) + 1);
	memcpy(player->name, name, strlen(name) + 1);
	player->opponent = NULL;
	/* contents will be initialized by new_game */
	player->contents = NULL;
	player->view = malloc(sizeof(struct game_view_data));
	list_append(player_list, player);
	fprintf(logging_fp, "player_create: Created player '%s'\n",
	        player->name);
	player_get_by_name(player->name);
	fprintf(logging_fp, "player_create: There are now %d players\n",
	        player_list->length);

	new_game(&player->contents);

	return player;
}

void player_set_opponent(Player *player, Player *opponent) {
	if (opponent == NULL)
		return;

	player->opponent = opponent;
	opponent->opponent = player;

	fprintf(logging_fp,
	        "player_set_opponent: %s and %s are now opponents\n",
	        player->name, opponent->name);
}
