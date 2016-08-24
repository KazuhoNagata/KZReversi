/***************************************************************************
* Name  : hash.cpp
* Brief : 置換表関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#include <stdlib.h>
#include "cpu.h"
#include "hash.h"
#include "eval.h"
#include "move.h"
#include "mt.h"

int freeFlag = TRUE;

UINT64 g_hashBoard[2][64];
UINT64 g_colorHash[2];

/*
* ハッシュテーブルを解放する。
* パラメータ：
* hash_table 解放するハッシュテーブル
*/
void hash_free(HashTable* hash_table){
	free(hash_table->entry);
	hash_table->entry = NULL;
}


static void HashFinalize(HashTable *hash)
{
	if (freeFlag == TRUE){
		return;
	}
	if (hash->entry) {
		free(hash->entry);
	}
	freeFlag = TRUE;
}

void HashDelete(HashTable *hash)
{
	HashFinalize(hash);
	free(hash);
}

void HashClear(HashTable *hash)
{
	if (hash != NULL) memset(hash->entry, 0, sizeof(HashEntry) * hash->size);
}

static int HashInitialize(HashTable *hash, int in_size)
{
	memset(hash, 0, sizeof(HashTable));
	hash->size = in_size;
	hash->entry = (HashEntry *)malloc(sizeof(HashEntry) * in_size);

	if (!hash->entry) {
		return FALSE;
	}

	HashClear(hash);

	return TRUE;
}

HashTable *HashNew(UINT32 in_size)
{
	HashTable *hash;
	freeFlag = FALSE;
	hash = (HashTable *)malloc(sizeof(HashTable));
	if (hash) {
		if (!HashInitialize(hash, in_size)) {
			HashDelete(hash);
			hash = NULL;
		}
	}
	return hash;
}

void HashSet(HashTable *hash, int hashValue, const HashInfo *in_info)
{
	memcpy(&hash->entry[hashValue], in_info, sizeof(HashInfo));
}

HashInfo *HashGet(HashTable *hash, int key, UINT64 bk, UINT64 wh)
{
	HashEntry *hash_entry;

	if (hash != NULL) {
		hash_entry = &hash->entry[key];
		if (hash_entry->deepest.bk == bk && hash_entry->deepest.wh == wh)
			return &(hash_entry->deepest);
		if (hash_entry->newest.bk == bk && hash_entry->newest.wh == wh)
			return &(hash_entry->newest);
	}

	return NULL;

}



void HashUpdate(
	HashTable* hash_table,
	UINT32 key,
	UINT64 bk,
	UINT64 wh,
	INT32 alpha,
	INT32 beta,
	INT32 score,
	INT32 depth,
	INT32 move,
	INT32 selectivity,
	INT32 inf_score)
{
	HashEntry *hash_entry;
	HashInfo *deepest, *newest;

	if (!g_tableFlag || hash_table == NULL) return;
	hash_entry = &(hash_table->entry[key]);
	deepest = &(hash_entry->deepest);
	newest = &(hash_entry->newest);

	if (score >= beta)
	{
		// [score, +INF]
		// try to update first hash
		if (deepest->bk == bk && deepest->wh == wh && deepest->selectivity <= selectivity)
		{
			deepest->bestmove = move;
			deepest->depth = depth;
			deepest->lower = score;
			deepest->upper = inf_score;
			deepest->selectivity = selectivity;
		}
		// try to update next hash
		else if (newest->bk == bk && newest->wh == wh && newest->selectivity <= selectivity)
		{
			newest->bestmove = move;
			newest->depth = depth;
			newest->lower = score;
			newest->upper = inf_score;
			newest->selectivity = selectivity;
		}
		// try to entry first hash
		else if (deepest->selectivity <= selectivity)
		{
			// deepest already entried?
			if (deepest->bk != 0 || deepest->wh != 0)
			{
				deepest = newest;
			}
			deepest->bk = bk;
			deepest->wh = wh;
			deepest->bestmove = move;
			deepest->depth = depth;
			deepest->lower = score;
			deepest->upper = inf_score;
			deepest->selectivity = selectivity;
		}
	}
	else if (score > alpha)
	{
		// [score, score]
		if (deepest->bk == bk && deepest->wh == wh && deepest->selectivity <= selectivity)
		{
			deepest->bestmove = move;
			deepest->depth = depth;
			deepest->lower = score;
			deepest->upper = score;
			deepest->selectivity = selectivity;
		}
		// try to update next hash
		else if (newest->bk == bk && newest->wh == wh && newest->selectivity <= selectivity)
		{
			newest->bestmove = move;
			newest->depth = depth;
			newest->lower = score;
			newest->upper = score;
			newest->selectivity = selectivity;
		}
		// try to entry first hash
		else if (deepest->selectivity <= selectivity)
		{
			// deepest already entried?
			if (deepest->bk != 0 || deepest->wh != 0)
			{
				deepest = newest;
			}
			deepest->bk = bk;
			deepest->wh = wh;
			deepest->bestmove = move;
			deepest->depth = depth;
			deepest->lower = score;
			deepest->upper = score;
			deepest->selectivity = selectivity;
		}
	}
	else
	{
		// [-INF, score]
		if (deepest->bk == bk && deepest->wh == wh && deepest->selectivity <= selectivity)
		{
			deepest->bestmove = move;
			deepest->depth = depth;
			deepest->lower = -inf_score;
			deepest->upper = score;
			deepest->selectivity = selectivity;
		}
		// try to update next hash
		else if (newest->bk == bk && newest->wh == wh && newest->selectivity <= selectivity)
		{
			newest->bestmove = move;
			newest->depth = depth;
			newest->lower = -inf_score;
			newest->upper = score;
			newest->selectivity = selectivity;
		}
		// try to entry first hash
		else if (deepest->selectivity <= selectivity)
		{
			// deepest already entried?
			if (deepest->bk != 0 || deepest->wh != 0)
			{
				// try to entry second hash
				deepest = newest;
			}
			deepest->bk = bk;
			deepest->wh = wh;
			deepest->bestmove = move;
			deepest->depth = depth;
			deepest->lower = -inf_score;
			deepest->upper = score;
			deepest->selectivity = selectivity;
		}
	}
}

void FixTableToMiddle(HashTable *hash)
{
	for (int i = 0; i < hash->size; i++)
	{
		hash->entry[i].deepest.lower = NEGAMIN;
		hash->entry[i].newest.lower = NEGAMIN;
		hash->entry[i].deepest.upper = NEGAMAX;
		hash->entry[i].newest.upper = NEGAMAX;
		hash->entry[i].deepest.selectivity = 0;
		hash->entry[i].newest.selectivity = 0;
	}

}

void FixTableToWinLoss(HashTable *hash)
{
	for (int i = 0; i < hash->size; i++)
	{
		hash->entry[i].deepest.lower = -(WIN + 1);
		hash->entry[i].newest.lower = -(WIN + 1);
		hash->entry[i].deepest.upper = (WIN + 1);
		hash->entry[i].newest.upper = (WIN + 1);
		hash->entry[i].deepest.selectivity = 0;
		hash->entry[i].newest.selectivity = 0;
	}
}

void FixTableToExact(HashTable *hash)
{

	for (int i = 0; i < hash->size; i++)
	{
		hash->entry[i].deepest.lower = -INF_SCORE;
		hash->entry[i].newest.lower = -INF_SCORE;
		hash->entry[i].deepest.upper = INF_SCORE;
		hash->entry[i].newest.upper = INF_SCORE;
		hash->entry[i].deepest.depth = -1;
		hash->entry[i].newest.depth = -1;
		hash->entry[i].deepest.selectivity = 0;
		hash->entry[i].newest.selectivity = 0;
	}
}

void InitHashBoard()
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			g_hashBoard[i][j] = (((UINT64)genrand_int32()) << 32) | genrand_int32();
		}
	}

	g_colorHash[0] = (((UINT64)genrand_int32()) << 32) | genrand_int32();
	g_colorHash[1] = (((UINT64)genrand_int32()) << 32) | genrand_int32();
}

UINT32 GenerateHashValue(UINT64 bk, UINT64 wh, UINT32 color)
{
	UINT64 hashValue = 0;

	for (int j = 0; j < 64; j++)
	{
		hashValue ^= g_hashBoard[0][j] * ((bk & (1ULL << j)) >> j);
		hashValue ^= g_hashBoard[1][j] * ((wh & (1ULL << j)) >> j);
	}

	hashValue ^= g_colorHash[color];

	return (UINT32)(hashValue % g_casheSize);

}