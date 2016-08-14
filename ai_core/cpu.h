/***************************************************************************
* Name  : search.h
* Brief : 探索の処理全般を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#include "hash.h"

//#define LOSSGAME

#define KEY_HASH_MACRO(b, w, c) (UINT32)((b ^ ((w - c) >> 1ULL)) % (g_casheSize - 1))
//#define KEY_HASH_MACRO(b, w, c) GenerateHashValue(b, w, c);

#define ILLIGAL_ARGUMENT 0x80000001
#define MOVE_PASS 0x0

#define ON_MIDDLE 0
#define ON_WINLOSS 1
#define ON_EXACT 2

#define SOLVE_WLD   0
#define SOLVE_EXACT 1
#define SOLVE_MIDDLE 2

#define EMPTIES_MID_ORDER_TO_END_ORDER 12
#define EMPTIES_DEEP_TO_SHALLOW_SEARCH 8
#define DEPTH_DEEP_TO_SHALLOW_SEARCH 5

#define NO_PASS 0
#define ABORT 0x80000000

#define MPC_MIN_DEPTH 3

typedef struct
{
	UINT32 color;				// CPUの色
	UINT32 casheSize;			// 置換表のサイズ
	UINT32 searchDepth;			// 中盤読みの深さ
	UINT32 winLossDepth;		// 勝敗探索を開始する深さ
	UINT32 exactDepth;			// 石差探索を開始する深さ
	BOOL   bookFlag;			// 定石を使用するかどうか
	UINT32 bookVariability;	    // 定石の変化度
	BOOL   mpcFlag;				// MPCを使用するかどうか
	BOOL   tableFlag;			// 置換表を使用するかどうか

}CPUCONFIG;


typedef void(__stdcall *SetMessageToGUI)(char *);
extern SetMessageToGUI g_set_message_funcptr[2];

/* MPC */
typedef struct
{
	int depth;
	int offset;
	int deviation;
}MPCINFO;


typedef struct PVLINE {
	int   cmove;          // Number of moves in the line.
	char argmove[60];  // The line.
} PVLINE;

extern INT32 g_pvline[60];
extern INT32 g_pvline_len;

extern int g_solveMethod;
extern BOOL g_tableFlag;
extern char g_cordinates_table[64][4];
extern INT32 g_limitDepth;
extern INT32 g_empty;
extern UINT64 g_casheSize;
extern MPCINFO mpcInfo[22];
extern UINT64 g_countNode;
extern HashTable *g_hash;

extern BOOL g_mpcFlag;
extern BOOL g_tableFlag;
// CPU AI情報
extern BOOL g_AbortFlag;
extern UINT64 g_countNode;
extern char g_AiMsg[128];
extern char g_PVLineMsg[256];
extern SetMessageToGUI g_set_message_funcptr[2];

/***************************************************************************
* Name  : GetMoveFromAI
* Brief : CPUの着手を探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手可能位置のビット列
****************************************************************************/
UINT64 GetMoveFromAI(UINT64 bk, UINT64 wh, UINT32 emptyNum, CPUCONFIG *cpuConfig);

/***************************************************************************
* Name  : AB_SearchNoPV
* Brief : 単純なαβ探索によって手を決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 探索しているノードの深さ
*         empty     : 空白マスの数
*         color     : 探索しているノードの色
*         alpha     : 
*         beta      :
*         passed    : パスしたかのフラグ
* Return: 評価値
****************************************************************************/
INT32 AB_SearchNoPV(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed);

/***************************************************************************
* Name  : AlphaBetaSearch
* Brief : MPCありの単純なαβ探索によって手を決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 探索しているノードの深さ
*         empty     : 空白マスの数
*         color     : 探索しているノードの色
*         alpha     :
*         beta      :
*         passed    : パスしたかのフラグ
* Return: 評価値
****************************************************************************/
INT32 AB_Search(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	INT32 alpha, INT32 beta, UINT32 passed, PVLINE *pline);

/***************************************************************************
* Name  : SetAbortFlag
* Brief : CPUの処理を中断する
****************************************************************************/
void SetAbortFlag();


void CreatePVLineStr(PVLINE *pline, INT32 empty, INT32 score);