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

int freeFlag = TRUE;


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
	INT32 empty,
	INT8 move,
	INT32 inf_score)
{
	HashEntry *hash_entry;
	HashInfo *deepest, *newest;

	if (!g_tableFlag || hash_table == NULL) return;

	// ハッシュエントリのアドレス
	hash_entry = &(hash_table->entry[key]);
	deepest = &(hash_entry->deepest);
	newest = &(hash_entry->newest);

	/* deepestエントリの更新を試みる */
	if (deepest->bk == bk && deepest->wh == wh)
	{
		if (score < beta && score < deepest->upper)
			deepest->upper = score;
		if (score > alpha && score > deepest->lower)
			deepest->lower = score;
		deepest->bestmove = move;
		/* newestエントリの更新を試みる */
	}
	else if (newest->bk == bk && newest->wh == wh)
	{
		if (score < beta && score < newest->upper)
			newest->upper = score;
		if (score > alpha && score > newest->lower)
			newest->lower = score;
		newest->bestmove = move;
		/* それ以外の場合でdeepestエントリの更新を試みる */
	}
	else if (deepest->empty < empty)
	{
		if (newest->empty < deepest->empty) *newest = *deepest;
		deepest->bk = bk;
		deepest->wh = wh;
		deepest->empty = empty;
		deepest->lower = -inf_score;
		deepest->upper = inf_score;
		if (score < beta) deepest->upper = score;
		if (score > alpha) deepest->lower = score;
		deepest->bestmove = move;
		/* それ以外の場合でnewestエントリを更新する */
	}
	else if (newest->empty < empty)
	{
		newest->bk = bk;
		newest->wh = wh;
		newest->empty = empty;
		newest->lower = -inf_score;
		newest->upper = inf_score;
		if (score < beta) newest->upper = score;
		if (score > alpha) newest->lower = score;
		newest->bestmove = move;
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
	}

}

void FixTableToWinLoss(HashTable *hash)
{
	for (int i = 0; i < hash->size; i++)
	{
		hash->entry[i].deepest.lower = -INF_SCORE;
		hash->entry[i].newest.lower = -INF_SCORE;
		hash->entry[i].deepest.upper = INF_SCORE;
		hash->entry[i].newest.upper = INF_SCORE;
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
		hash->entry[i].deepest.empty = 59;
		hash->entry[i].newest.empty = 59;
	}
}