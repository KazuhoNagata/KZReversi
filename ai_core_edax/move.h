/***************************************************************************
* Name  : move.h
* Brief : ’…ŽèŠÖ˜A‚ÌŒvŽZ‚ðs‚¤
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#pragma once

#include "const.h"

/** macro to iterate over the movelist */
#define foreach_move(iter, movelist) \
	for ((iter) = (movelist)->move->next; (iter); (iter) = (iter)->next)

/** macro to iterate over the movelist from best to worst move */
#define foreach_best_move(iter, movelist) \
	for ((iter) = movelist_best(movelist); (iter); (iter) = move_next_best(iter))

/** (simple) sequence of a legal moves */
typedef struct Line{
	char move[GAME_SIZE];   /**< array of a suite of moves */
	int n_moves;
	int color;
} Line;

/** move representation */
typedef struct Move {
	unsigned long long flipped;   /**< bitboard representation of flipped squares */
	int x;                        /**< square played */
	int score;                    /**< score for this move */
	unsigned int cost;            /**< move cost */
	struct Move *next;            /**< next move in a MoveList */
} Move;

/** (simple) list of a legal moves */
typedef struct MoveList {
	Move move[MAX_MOVE + 2];   /**< array of legal moves */
	int n_moves;
} MoveList;

UINT64 CreateMoves(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p);
void StoreMovelist(MoveList *list, UINT64 bk, UINT64 wh, UINT64 moves);

UINT64 GetPotentialMoves(UINT64 P, UINT64 O);
INT32 get_potential_mobility(const UINT64 P, const UINT64 O);
INT32 get_weighted_mobility(const UINT64 P, const UINT64 O);

BOOL boardMoves(UINT64 *bk, UINT64 *wh, UINT64 move, INT32 pos);

void  GenerateMoveList(MoveList *moves, UINT64 blank);
MoveList *UpdateMoveList(MoveList *moves, INT32 pos);
void  RestoreMoveList(MoveList *moves, INT32 pos, MoveList *idx);