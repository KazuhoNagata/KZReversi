#include "stdafx.h"


BOOL search_SC_NWS(UINT64 bk, UINT64 wh, INT32 empty, INT32 alpha, INT32 *score);

INT32 PVS_SearchDeepExact_YBWC(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);

INT32 PVS_SearchDeepExact(
	UINT64     bk,
	UINT64     wh,
	INT32      empty,
	INT32      depth,
	INT32      parity,
	UINT32     color,
	HashTable *hash,
	HashTable *pvHash,
	INT32      alpha,
	INT32      beta,
	UINT32     passed,
	INT32     *p_selectivity,
	PVLINE    *pline
);

INT32 AB_SearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, UINT32 color, 
	INT32 alpha, INT32 beta, UINT32 passed, INT32 quad_parity, INT32 *p_selectivity, PVLINE *pline);

INT32 PVS_SearchDeepWinLoss(UINT64 bk, UINT64 wh, INT32 empty, INT32 depth, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32* p_selectivity, PVLINE *pline);
INT32 AB_SearchWinLoss(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty,
	UINT32 color, INT32 alpha, INT32 beta, UINT32 passed, INT32* p_selectivity, PVLINE *pline);

BOOL TableCutOff(HashInfo *hashInfo, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 *alpha, INT32 *beta, INT32 *score, INT32 *bestmove, INT32 *selectivity
);

BOOL CheckTableCutOff(
	HashTable *hash, UINT32 *key, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 alpha, INT32 beta, INT32 *score
);

BOOL CheckTableCutOff_PV(
	HashTable *hash, UINT32 *key, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 alpha, INT32 beta, INT32 *score
);

BOOL ProbCutOffEnd(
	UINT64     bk,
	UINT64     wh,
	INT32      empty,
	UINT32     color,
	INT32      alpha,
	INT32      beta,
	UINT32     passed,
	INT32     *score
);