/***************************************************************************
* Name  : cpu_root.h
* Brief : íTçıÇÃèàóùëSî ÇçsÇ§
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#include "search.h"
#include "hash.h"
#include "cpu_root.h"
#include "ybwc.h"

/** Swith from endgame to shallow search (faster but less node efficient) at this depth. */
#define DEPTH_TO_SHALLOW_SEARCH 7

/** Switch from midgame to endgame search (faster but less node efficient) at this depth. */
#define DEPTH_MIDGAME_TO_ENDGAME 15

/** Switch from midgame result (evaluated score) to endgame result (exact score) at this number of empties. */
#define ITERATIVE_MIN_EMPTIES 10

/** Store bestmoves in the pv_hash up to this height. */
#define PV_HASH_HEIGHT 5

INT32 PVS_SearchMid(Search *search, INT32 depth, INT32 alpha, INT32 beta, Node *parent);

