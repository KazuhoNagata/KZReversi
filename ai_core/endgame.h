#include "stdafx.h"

#pragma once

BOOL search_SC_NWS(UINT64 bk, UINT64 wh, INT32 empty, INT32 alpha, INT32 *score);

INT32 PVS_SearchExact_YBWC(UINT64 bk, UINT64 wh, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);

INT32 PVS_SearchExact(
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
	PVLINE    *pline,
	UINT32     mpc_count
);

INT32 AB_SearchExact(UINT64 bk, UINT64 wh, UINT64 blank, INT32 empty, UINT32 color, 
	INT32 alpha, INT32 beta, UINT32 passed, INT32 quad_parity, INT32 *p_selectivity, PVLINE *pline);

INT32 PVS_SearchWinLoss(
	UINT64 bk,
	UINT64 wh,
	INT32 empty,
	INT32 depth,
	UINT32 parity,
	UINT32 color,
	HashTable *hash,
	HashTable *pvHash,
	INT32 alpha,
	INT32 beta,
	UINT32 passed,
	INT32* p_selectivity,
	PVLINE *pline
);

INT32 AB_SearchWinLoss(
	UINT64 bk,
	UINT64 wh,
	UINT64 blank,
	INT32 empty,
	UINT32 color,
	INT32 alpha,
	INT32 beta,
	UINT32 passed,
	UINT32 quad_parity,
	INT32 *p_selectivity,
	PVLINE *pline
);

BOOL TableCutOff(HashInfo *hashInfo, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 *alpha, INT32 *beta, INT32 *score, INT32 *bestmove, INT32 *selectivity
);

HashInfo *CheckTableCutOff(
	HashTable *hash, UINT32 *key, UINT64 bk, UINT64 wh, UINT32 color, INT32 empty,
	INT32 alpha, INT32 beta, INT32 *score
);

HashInfo *CheckTableCutOff_PV(
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
	INT32     *score,
	UINT32     mpc_count
);