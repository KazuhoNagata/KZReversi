/***************************************************************************
* Name  : hash.h
* Brief : 置換表関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

/* 置換表定義 */
struct HashInfo
{
	UINT64 b_board;
	UINT64 w_board;
	INT32 lower;
	INT32 upper;
	INT8 bestmove;
	INT8 depth;
	INT8 move_cnt;
	INT8 locked;
	INT8 bestLocked;
};

struct HashTable
{
	INT32 num;
	HashInfo *data;
	INT32 getNum;
	INT32 hitNum;
};

#define PREPARE_LOCKED 3
#define LOCKED 2

HashTable *HashNew(UINT32 in_size);
void HashDelete(HashTable *hash);
void HashClear(HashTable *hash);

void HashSet(HashTable *hash, int hashValue, const HashInfo *in_info);
INT32 HashGet(HashTable *hash, int hashValue, UINT64 b_board, UINT64 w_board, HashInfo *out_info);

void HashUpdate(
	HashInfo *hash_info,
	INT8 bestmove,
	UINT32 depth,
	INT32 max,
	INT32 alpha,
	INT32 beta,
	INT32 lower,
	INT32 upper);

void HashCreate(
	HashInfo *hash_info,
	UINT64 b_board,
	UINT64 w_board,
	INT32 bestmove,
	INT32 move_cnt,
	UINT32 depth,
	INT32 max,
	INT32 alpha, INT32 beta,
	INT32 lower, INT32 upper);

void FixTableToMiddle(HashTable *hash);
void FixTableToWinLoss(HashTable *hash);
void FixTableToExact(HashTable *hash);