/***************************************************************************
* Name  : search.h
* Brief : 探索処理のパラメータヘッダ
* Date  : 2020/03/19
****************************************************************************/

#include "stdafx.h"
#pragma once

#include "hash.h"
#include "search.h"
#include "move.h"
#include "board.h"
#include "const.h"
#include "empty.h"

#define ITERATIVE_MIN_EMPTIES 10

/** Selectivity probcut */
typedef struct Selectivity {
	double t; /**< selectivity value */
	int level; /**< level of selectivity */
	int percent; /**< selectivity value as a percentage */
} Selectivity;

struct Task;
struct TaskQueue;

/** Bound */
typedef struct Bound {
	int lower;
	int upper;
} Bound;


/** Result */
typedef struct Result {
	int depth;                   /**< searched depth */
	int selectivity;             /**< searched selectivity */
	int move;                    /**< best move found */
	int score;                   /**< best score */
	Bound bound[BOARD_SIZE + 2]; /**< score bounds / move */
	Line pv[1];                  /**< principal variation */
	long long time;              /**< searched time */
	unsigned long long n_nodes;  /**< searched node count */
	bool book_move;              /**< book move origin */
	int n_moves;                 /**< total moves to search */
	int n_moves_left;            /**< left moves to search */
	SpinLock spin;
} Result;

typedef struct Search
{
	Board board[1];                               /**< othello board */
	SquareList empties[BOARD_SIZE + 2];           /**< list of empty squares */
	SquareList *x_to_empties[BOARD_SIZE + 2];     /**< link position to the list */
	int n_empties;                                /**< number of empty squares */
	int player;                                   /**< player color */
	int id;                                       /**< search id */

	HashTable hash_table[1];                      /**< hashtable */
	HashTable pv_table[1];                        /**< hashtable for the pv */
	HashTable shallow_table[1];                   /**< hashtable for short search */
	UINT8     eval[64];                           /**< eval */

	struct TaskStack *tasks;                      /**< available task queue */
	struct Task *task;                            /**< search task */
	SpinLock spin;                                /**< search lock */
	struct Search *parent;                        /**< parent search */
	struct Search *child[MAX_THREADS];            /**< child search */
	struct Search *master;                        /**< master search (parent of all searches)*/
	volatile int n_child;                         /**< search child number */

	int depth;                                    /**< depth level */
	int selectivity;                              /**< selectivity level */
	int probcut_level;                            /**< probcut recursivity level */
	unsigned int parity;                          /**< parity */
	int depth_pv_extension;                       /**< depth for pv_extension */
	volatile Stop stop;                           /**< thinking status */
	bool allow_node_splitting;                    /**< allow parallelism */
	struct {
		long long  extra;                         /**< extra alloted time */
		volatile long long spent;                 /**< time spent thinking */
		bool extended;                            /**< flag to extend time only once */
		bool can_update;                          /**< flag allowing to extend time */
		long long  mini;                          /**< minimal alloted time */
		long long  maxi;                          /**< maximal alloted time */
	} time;                                       /**< time */
	MoveList movelist[1];                         /**< list of moves */
	int height;                                   /**< search height from root */
	NodeType node_type[GAME_SIZE];                /**< node type (pv node, cut node, all node) */
	Bound stability_bound;                        /**< score bounds according to stable squares */

	struct {
		int depth;                                /**< depth */
		int selectivity;                          /**< final selectivity */
		long long time;                           /**< time in sec. */
		bool time_per_move;                       /**< time_per_move or per game ?*/
		int verbosity;                            /**< verbosity level */
		bool keep_date;                           /**< keep date */
		const char *header;                       /**< header for search output */
		const char *separator;                    /**< separator for search output */
		bool guess_pv;                            /**< guess PV (in cassio mode only) */
		int multipv_depth;                        /**< multi PV depth */
		int hash_size;                            /**< hashtable size */
	} options;                                    /**< local (threadable) options. */

	Result *result;                               /**< shared result */

	void(*observer)(Result*);                     /**< call back function to print search result */

	volatile unsigned long long n_nodes;          /**< node counter */
	volatile unsigned long long child_nodes;      /**< node counter */
}Search;


extern const INT32 QUADRANT_ID[];
extern const Selectivity selectivity_table[];
extern const INT32 NO_SELECTIVITY;
extern const INT32 NWS_STABILITY_THRESHOLD[];
extern const INT32 PVS_STABILITY_THRESHOLD[];
extern const INT32 SQUARE_TYPE[];

/**
 * @brief Return the number of nodes searched.
 *
 * @param search  Search.
 * @return node count.
 */
UINT64 search_count_nodes(Search *search);

void search_set_task_number(Search*, const int);

void search_swap_parity(Search*, const int);
void search_get_movelist(const Search*, MoveList*);
void search_update_endgame(Search*, const Move*);
void search_restore_endgame(Search*, const Move*);
void search_pass_endgame(Search*);
void search_update_midgame(Search*, const Move*);
void search_restore_midgame(Search*, const Move*);
void search_update_pass_midgame(Search*);
void search_restore_pass_midgame(Search*);
void search_observer(Result*);
void search_get_movelist(const Search *search, MoveList *movelist);
void search_init(Search *search, CPUCONFIG *config);

void record_best_move(Search *search, const Board *init_board, const Move *bestmove, const int alpha, const int beta, const int depth);