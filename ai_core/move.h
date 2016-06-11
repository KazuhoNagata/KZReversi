/***************************************************************************
* Name  : move.h
* Brief : ’…èŠÖ˜A‚ÌŒvZ‚ğs‚¤
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#define MOVE_NONE 0xFF
#define NOMOVE  -1
#define PASS -2

/*! move representation */
typedef struct Move {
	UINT32 pos;
	UINT64 rev;                     /*< flipped discs */
	INT64 score;                   /*< score for this move */
} Move;

/*! (simple) list of a legal moves */
typedef struct MoveList {
	Move move;                /*!< a legal move */
	struct MoveList *next;    /*!< link to next legal move */
} MoveList;

UINT64 CreateMoves(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p);
void StoreMovelist(MoveList *list, UINT64 bk, UINT64 wh, UINT64 moves);

UINT64 GetPotentialMoves(UINT64 P, UINT64 O, UINT64 blank);
UINT64 GetFirstRandomMove(UINT64 move);