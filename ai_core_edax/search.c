
#include "bit64.h"
#include "search.h"
#include "move.h"
#include "ybwc.h"
#include "eval.h"

/** level with no selectivity */
const int NO_SELECTIVITY = 5;

/** predefined selectivity */
const Selectivity selectivity_table[] = {
	{1.1, 0, 73}, // strong selectivity
	{1.5, 1, 87}, //       |
	{2.0, 2, 95}, //       |
	{2.6, 3, 98}, //       |
	{3.3, 4, 99}, //       V
	{999, 5,100}, // no selectivity
};

/**
 * @brief Return the number of nodes searched.
 *
 * @param search  Search.
 * @return node count.
 */
UINT64 search_count_nodes(Search *search)
{
	return search->n_nodes + search->child_nodes;
}

/**
 * @brief default observer.
 *
 * @param result search results to print.
 */
void search_observer(Result *result)
{
	//result_print(result, stdout);
	//putchar('\n');
}

void search_resize_hashtable(Search *search, CPUCONFIG *config)
{
	if (search->options.hash_size != config->casheSize)
	{
		const int hash_size = 1u << config->casheSize;
		const int pv_size = hash_size > 16 ? hash_size >> 4 : 16;

		hash_init(search->hash_table, hash_size);
		hash_init(search->pv_table, pv_size);
		hash_init(search->shallow_table, hash_size);
		search->options.hash_size = config->casheSize;
	}
}

/**
 * @brief Init the *main* search.
 *
 * Initialize a new search structure.
 * @param search  search.
 */
void search_init(Search *search, CPUCONFIG *config)
{
	/* id */
	search->id = 0;

	/* running state */
	search->stop = STOP_END;

	/* hash_table */
	search->options.hash_size = config->casheSize;
	search->hash_table->hash = g_hash;
	search->hash_table->hash_mask = 0;
	search->pv_table->hash = g_pvHash;
	search->pv_table->hash_mask = 0;
	search->shallow_table->hash = g_shHash;
	search->shallow_table->hash_mask = 0;
	search_resize_hashtable(search, config);

	/* board */
	search->board->player = search->board->opponent = 0;
	search->player = EMPTY;

	/* evaluation function */
	eval_init(search->eval);

	// radom generator
	//random_seed(search->random, real_clock());

	/* task stack */
	search->tasks = (TaskStack*)malloc(sizeof(TaskStack));
	if (search->tasks == NULL) {
		fatal_error("Cannot allocate a task stack\n");
	}
	//if (g_options.cpu_affinity) thread_set_cpu(thread_self(), 0);
	task_stack_init(search->tasks, config->n_task);
	search->allow_node_splitting = (search->tasks->n > 1);

	/* task associated with the current search */
	search->task = search->tasks->task;
	search->task->loop = false;
	search->task->run = true;
	search->task->node = NULL;
	search->task->move = NULL;
	search->task->n_calls = 0;
	search->task->n_nodes = 0;
	search->task->search = search;

	search->parent = NULL;
	search->n_child = 0;
	search->master = search; /* main search */

	/* lock */
	spin_init(search);

	/* result */
	search->result = (Result*)malloc(sizeof(Result));
	if (search->result == NULL) {
		fatal_error("Cannot allocate a task stack\n");
	}
	spin_init(search->result);
	search->result->move = NOMOVE;

	search->n_nodes = 0;
	search->child_nodes = 0;


	/* observers */
	search->observer = search_observer;

	/* options */
	search->options.depth = 60;
	search->options.selectivity = NO_SELECTIVITY;
	search->options.time = TIME_MAX;
	search->options.time_per_move = false;
	search->options.verbosity = 0;
	search->options.keep_date = false;
	search->options.header = NULL;
	search->options.separator = NULL;
	search->options.guess_pv = false;
	search->options.multipv_depth = 0;

	//log_open(search_log, options.search_log_file);
}

/**
 * @brief Free the search allocated ressource.
 *
 * Free a previously initialized search structure.
 * @param search search.
 */
void search_free(Search *search)
{

	hash_free(search->hash_table);
	hash_free(search->pv_table);
	hash_free(search->shallow_table);
	eval_free(search->eval);

	task_stack_free(search->tasks);
	free(search->tasks);
	spin_free(search);

	spin_free(search->result);
	free(search->result);

	//log_close(search_log);
}

/**
 * @brief Set up various structure once the board has been set.
 *
 * Initialize the list of empty squares, the parity and the evaluation function.
 * @param search search.
 */
void search_setup(Search *search)
{
	int i;
	SquareList *empty;
	const int presorted_x[] = {
		A1, A8, H1, H8,                    /* Corner */
		C4, C5, D3, D6, E3, E6, F4, F5,    /* E */
		C3, C6, F3, F6,                    /* D */
		A3, A6, C1, C8, F1, F8, H3, H6,    /* A */
		A4, A5, D1, D8, E1, E8, H4, H5,    /* B */
		B4, B5, D2, D7, E2, E7, G4, G5,    /* G */
		B3, B6, C2, C7, F2, F7, G3, G6,    /* F */
		A2, A7, B1, B8, G1, G8, H2, H7,    /* C */
		B2, B7, G2, G7,                    /* X */
		D4, E4, D5, E5,                    /* center */
	};

	Board *board = search->board;
	unsigned long long E;

	// init empties
	search->n_empties = 0;

	empty = search->empties;
	empty->x = NOMOVE; /* sentinel */
	empty->previous = NULL;
	empty->next = empty + 1;
	empty = empty->next;
	E = ~(board->player | board->opponent);
	for (i = 0; i < BOARD_SIZE; ++i) {    /* add empty squares */
		if ((E & x_to_bit(presorted_x[i]))) {
			empty->x = presorted_x[i];
			empty->b = x_to_bit(presorted_x[i]);
			empty->quadrant = QUADRANT_ID[empty->x];
			empty->previous = empty - 1;
			empty->next = empty + 1;
			search->x_to_empties[presorted_x[i]] = empty;
			empty = empty->next;
			++search->n_empties;
		}
	}
	empty->x = NOMOVE; /* sentinel */
	empty->b = 0;
	empty->previous = empty - 1;
	empty->next = NULL;

	empty = search->empties + PASS;
	empty->x = PASS;
	empty->b = 0;
	empty->previous = empty->next = empty;
	search->x_to_empties[PASS] = empty;

	empty = search->empties + NOMOVE;
	empty->x = NOMOVE;
	empty->b = 0;
	empty->previous = empty->next = empty;
	search->x_to_empties[NOMOVE] = empty;

	// init parity
	search->parity = 0;
	foreach_empty(empty, search->empties) {
		search->parity ^= empty->quadrant;
	}

	// init the evaluation function
	eval_set(search->eval, board);
}

/**
 * @brief Clone a search for parallel search.
 *
 * @param search search.
 * @param master search to be cloned.
 */
void search_clone(Search *search, Search *master)
{
	search->stop = STOP_END;
	search->player = master->player;
	*search->board = *master->board;
	search_setup(search);
	*search->hash_table = *master->hash_table; // share the hashtable
	*search->pv_table = *master->pv_table; // share the pvtable
	*search->shallow_table = *master->shallow_table; // share the pvtable
	search->tasks = master->tasks;
	search->observer = master->observer;

	search->depth = master->depth;
	search->selectivity = master->selectivity;
	search->probcut_level = master->probcut_level;
	search->depth_pv_extension = master->depth_pv_extension;
	search->time = master->time;
	search->height = master->height;
	search->allow_node_splitting = master->allow_node_splitting;
	search->node_type[search->height] = master->node_type[search->height];
	search->options = master->options;
	search->result = master->result;
	search->n_nodes = 0;
	search->child_nodes = 0;
	search->stability_bound = master->stability_bound;
	spin_lock(master);
	assert(master->n_child < MAX_THREADS);
	master->child[master->n_child++] = search;
	spin_unlock(master);
	search->parent = master;
	search->master = master->master;
}

/**
 * @brief Clean-up some search data.
 *
 * @param search search.
 */
void search_cleanup(Search *search)
{
	hash_cleanup(search->hash_table);
	hash_cleanup(search->pv_table);
	hash_cleanup(search->shallow_table);
}


/**
 * @brief Set the board to analyze.
 *
 * @param search search.
 * @param board board.
 * @param player player's turn.
 */
void search_set_board(Search *search, const Board *board, const int player)
{
	search->player = player;
	*search->board = *board;
	search_setup(search);
	search_get_movelist(search, search->movelist);
}

#if 0
/**
 * @brief Set the search level.
 *
 * Compute the couple (depth, selectivity) as a function of (level, n_empties)
 * @param search search.
 * @param level  search level.
 * @param n_empties Search stage.
 */
void search_set_level(Search *search, const int level, const int n_empties)
{
	search->options.depth = LEVEL[level][n_empties].depth;
	search->options.selectivity = LEVEL[level][n_empties].selectivity;

}
#endif


/**
 * @brief Change the number of task.
 *
 * @param search Search.
 * @param n New task number.
 */
void search_set_task_number(Search *search, const int n)
{
	assert(n > 0 && n < MAX_THREADS);
	task_stack_resize(search->tasks, n);
	search->allow_node_splitting = (n > 1);
}

/**
 * @brief Change parity.
 *
 * @param search Search.
 * @param x      Played square.
 */
void search_swap_parity(Search *search, const int x)
{
	search->parity ^= QUADRANT_ID[x];
}

/**
 * @brief Get a list of legal moves.
 *
 * Compute the complete list of legal moves and store it into a simple linked
 * list, to fasten ulterior move sorting.
 * Note: at this point the list is sorted from A1 to H8 (i.e. unsorted).
 *
 * @param search Search.
 * @param movelist List of moves.
 */
void search_get_movelist(const Search *search, MoveList *movelist)
{
	Move *previous = movelist->move;
	Move *move = movelist->move + 1;
	const Board *board = search->board;
	unsigned long long moves = get_moves(board->player, board->opponent);
	register int x;

	foreach_bit(x, moves) {
		board_get_move(board, x, move);
		move->cost = 0;
		previous = previous->next = move;
		++move;
	}
	previous->next = NULL;
	movelist->n_moves = move - movelist->move - 1;
	assert(movelist->n_moves == get_mobility(board->player, board->opponent));
}

/**
 * @brief Update the search state after a move.
 *
 * @param search  search.
 * @param move    played move.
 */
void search_update_endgame(Search *search, const Move *move)
{
	search_swap_parity(search, move->x);
	empty_remove(search->x_to_empties[move->x]);
	board_update(search->board, move);
	--search->n_empties;

}

/**
 * @brief Restore the search state as before a move.
 *
 * @param search  search.
 * @param move    played move.
 */
void search_restore_endgame(Search *search, const Move *move)
{
	search_swap_parity(search, move->x);
	empty_restore(search->x_to_empties[move->x]);
	board_restore(search->board, move);
	++search->n_empties;
}

/**
 * @brief Update the search state after a passing move.
 *
 * @param search  search.
 */
void search_pass_endgame(Search *search)
{
	board_pass(search->board);
}


//static Line debug_line;

/**
 * @brief Update the search state after a move.
 *
 * @param search  search.
 * @param move    played move.
 */
void search_update_midgame(Search *search, const Move *move)
{
	static const NodeType next_node_type[] = { CUT_NODE, ALL_NODE, CUT_NODE };

	//	line_push(&debug_line, move->x);

	search_swap_parity(search, move->x);
	empty_remove(search->x_to_empties[move->x]);
	board_update(search->board, move);
	//eval_update(search->eval, move);
	--search->n_empties;
	++search->height;
	search->node_type[search->height] = next_node_type[search->node_type[search->height - 1]];
}

/**
 * @brief Restore the search state as before a move.
 *
 * @param search  search.
 * @param move    played move.
 */
void search_restore_midgame(Search *search, const Move *move)
{
	//	line_print(&debug_line, 100, " ", stdout); putchar('\n');
	//	line_pop(&debug_line);

	search_swap_parity(search, move->x);
	empty_restore(search->x_to_empties[move->x]);
	board_restore(search->board, move);
	//eval_restore(search->eval, move);
	++search->n_empties;
	assert(search->height > 0);
	--search->height;
}

/**
 * @brief Update the search state after a passing move.
 *
 * @param search  search.
 */
void search_update_pass_midgame(Search *search)
{
	static const NodeType next_node_type[] = { CUT_NODE, ALL_NODE, CUT_NODE };

	board_pass(search->board);
	//eval_pass(search->eval);
	++search->height;
	search->node_type[search->height] = next_node_type[search->node_type[search->height - 1]];
}

/**
 * @brief Update the search state after a passing move.
 *
 * @param search  search.
 */
void search_restore_pass_midgame(Search *search)
{
	board_pass(search->board);
	//eval_pass(search->eval);
	//assert(search->height > 0);
	--search->height;
}

/**
 * @brief Compute the pv_extension.
 *
 * @param depth Depth.
 * @param n_empties Game stage (number of empty squares).
 */
int get_pv_extension(const int depth, const int n_empties)
{
	int depth_pv_extension;

	if (depth >= n_empties || depth <= 9) depth_pv_extension = -1;
	else if (depth <= 12) depth_pv_extension = 10; // depth + 8
	else if (depth <= 18) depth_pv_extension = 12; // depth + 10
	else if (depth <= 24) depth_pv_extension = 14; // depth + 12
	else depth_pv_extension = 16; // depth + 14

	return depth_pv_extension;
}

/**
 * @brief Check if final score use pv_extension or is solved.
 *
 * @param depth search depth.
 * @param n_empties position depth.
 * @return true if the score should be a final score.
 */
bool is_depth_solving(const int depth, const int n_empties)
{
	return (depth >= n_empties)
		|| (depth > 9 && depth <= 12 && depth + 8 >= n_empties)
		|| (depth > 12 && depth <= 18 && depth + 10 >= n_empties)
		|| (depth > 18 && depth <= 24 && depth + 12 >= n_empties)
		|| (depth > 24 && depth + 14 >= n_empties);
}




/**
 * @brief Stability Cutoff (TC).
 *
 * @param search Current position.
 * @param alpha Alpha bound.
 * @param score Score to return in case of a cutoff is found.
 * @return 'true' if a cutoff is found, false otherwise.
 */
bool search_SC_NWS(Search *search, const int alpha, int *score)
{
	const Board *board = search->board;

	if (alpha >= NWS_STABILITY_THRESHOLD[search->n_empties])
	{
		*score = SCORE_MAX - 2 * get_stability(board->opponent, board->player);
		if (*score <= alpha) return true;
	}
	return false;
}


/**
* @brief Stability Cutoff (SC).
*
* @param search Current position.
* @param alpha Alpha bound.
* @param beta Beta bound, to adjust if necessary.
* @param score Score to return in case of a cutoff is found.
* @return 'true' if a cutoff is found, false otherwise.
*/
BOOL search_SC_PVS(UINT64 bk, UINT64 wh, INT32 empty,
	volatile INT32 *alpha, volatile INT32 *beta, INT32 *score)
{
	if (*beta >= PVS_STABILITY_THRESHOLD[empty]) {
		*score = (64 - 2 * get_stability(wh, bk)) * EVAL_ONE_STONE;
		if (*score <= *alpha) {
			return TRUE;
		}
		else if (*score < *beta) *beta = *score;
	}
	return FALSE;
}