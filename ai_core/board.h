/***************************************************************************
* Name  : board.h
* Brief : 盤面関連の処理を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#define BLACK 0
#define WHITE 1

#define BK_FIRST 34628173824
#define WH_FIRST 68853694464

extern UINT8 board_parity[];
extern UINT8 board_parity_bit[];
extern UINT64 quad_parity_bitmask[];
extern const INT32 NWS_STABILITY_THRESHOLD[];
extern const INT32 PVS_STABILITY_THRESHOLD[];

typedef struct
{
	INT32 position; // 空きマスの位置
	UINT8 parity;   // 4x4の偶奇
}EmptyPosition;

struct EmptyList
{
	EmptyPosition empty;
	EmptyList *next;
};

extern UCHAR g_board[64];

void edge_stability_init(void);
int get_stability(const unsigned long long P, const unsigned long long O);
int get_edge_stability(const unsigned long long P, const unsigned long long O);
void create_quad_parity(UINT32 *q_parity, UINT64 blank);
void InitIndexBoard(UINT64 bk, UINT64 wh);
void Swap(UINT64 *bk, UINT64 *wh);
void create_empty_list(EmptyList *start, UINT64 blank);

UINT64 rotate_90(UINT64 board);
UINT64 rotate_180(UINT64 board);
UINT64 rotate_270(UINT64 board);
UINT64 symmetry_x(UINT64 board);
UINT64 symmetry_y(UINT64 board);
UINT64 symmetry_b(UINT64 board);
UINT64 symmetry_w(UINT64 board);