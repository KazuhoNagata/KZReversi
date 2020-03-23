/***************************************************************************
* Name  : ordering.h
* Brief : 手の並び替え関連の処理を行う
* Date  : 2016/02/02
****************************************************************************/

#include "stdafx.h"

#pragma once

#include "hash.h"

/* 候補手リスト */

typedef struct {
	char move;
	UINT64 rev;
}MOVELIST;
