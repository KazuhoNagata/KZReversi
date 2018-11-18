/***************************************************************************
* Name  : ordering.h
* Brief : 手の並び替え関連の処理を行う
* Date  : 2016/02/02
****************************************************************************/

#include "stdafx.h"

#pragma once

/* 候補手リスト */

typedef struct {
	char move;
	UINT64 rev;
}MOVELIST;

UINT32 GetOrderPosition(UINT64 blank);
void SortMoveList(
	MoveList *movelist,
	UINT64 bk,
	UINT64 wh,
	HashTable *hash,
	HashTable *pvHash,
	HashInfo *hashInfo,
	INT32 empty,
	UINT32 parity,
	INT32 alpha,
	INT32 beta,
	UINT32 color,
	INT32 *selectivity
);
void SortMoveListTableMoveFirst(MoveList *movelist, int move);
void SortFastfirst(MoveList *movelist, UINT64 bk, UINT64 wh);
void SortPotentionalFastfirst(MoveList *movelist, UINT64 bk, UINT64 wh, UINT64 blank);

void SortMoveListMiddle(
	MoveList  *movelist,
	UINT64     bk,
	UINT64     wh,
	HashTable *hash,
	HashTable *pvHash,
	UINT32     empty,
	INT32      depth,
	INT32      alpha,
	INT32      beta,
	UINT32     color
);

void SortMoveListEnd(
	MoveList *movelist,
	UINT64 bk, UINT64 wh,
	HashTable *hash,
	UINT32 empty,
	INT32 alpha, INT32 beta,
	UINT32 color);