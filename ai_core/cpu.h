/***************************************************************************
* Name  : search.h
* Brief : 探索の処理全般を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#pragma once

#include "hash.h"

//#define LOSSGAME

#define KEY_HASH_MACRO(b, w, c) (UINT32)((b ^ ((w) >> 1ULL)) % (g_casheSize - 1))
#define KEY_HASH_MACRO_PV(b, w, c) (UINT32)((b ^ ((w) >> 1ULL)) % (g_pvCasheSize - 1))
//#define KEY_HASH_MACRO(b, w, c) GenerateHashValue(b, w, c);

#define ILLIGAL_ARGUMENT 0x80000001
#define MOVE_PASS 0x0

#define ON_MIDDLE 0
#define ON_WLD 1
#define ON_EXACT 2

#define SOLVE_WLD   0
#define SOLVE_EXACT 1
#define SOLVE_MIDDLE 2

#define EMPTIES_MID_ORDER_TO_END_ORDER 12
#define EMPTIES_DEEP_TO_SHALLOW_SEARCH 7
#define END_DEPTH_DEEP_TO_SHALLOW_SEARCH 19
#define DEPTH_DEEP_TO_SHALLOW_SEARCH 5

#define NO_PASS 0
#define ABORT 0x40000000

#define MPC_MIN_DEPTH 3
#define MPC_END_MIN_DEPTH 6
#define MPC_END_MAX_DEPTH 30

typedef struct
{
	UINT32 color;				// CPUの色
	UINT32 casheSize;			// 置換表のサイズ
	UINT32 searchDepth;			// 中盤読みの深さ
	UINT32 winLossDepth;		// 勝敗探索を開始する深さ
	UINT32 exactDepth;			// 石差探索を開始する深さ
	UINT32 bookVariability;	    // 定石の変化度
	UINT8  bookFlag;			// 定石を使用するかどうか
	UINT8  mpcFlag;				// MPCを使用するかどうか
	UINT8  tableFlag;			// 置換表を使用するかどうか

}CPUCONFIG;


typedef void(__stdcall *SetMessageToGUI)(char *);

/* MPC */
typedef struct
{
	int depth;
	int offset;
	int deviation;
}MPCINFO;


typedef struct PVLINE {
	int   cmove;          // Number of moves in the line.
	char argmove[64];  // The line.
} PVLINE;

extern INT32 g_pvline[64];
extern INT32 g_pvline_len;

extern int g_solveMethod;
extern BOOL g_tableFlag;
extern char g_cordinates_table[64][4];
extern INT32 g_limitDepth;
extern INT32 g_empty;
extern INT32 g_move;
extern UINT64 g_casheSize;
extern UINT64 g_pvCasheSize;
extern INT32 g_infscore;
extern MPCINFO mpcInfo[22];
extern MPCINFO mpcInfo_end[26];
extern double MPC_CUT_VAL;
extern double MPC_END_CUT_VAL;
extern INT32 g_mpc_level;
const extern double cutval_table[8];
extern const INT32 g_max_cut_table_size;
extern UINT64 g_countNode;
extern HashTable *g_hash;
extern HashTable *g_pvHash;
extern INT32 g_key;

extern BOOL g_mpcFlag;
extern BOOL g_tableFlag;
// CPU AI情報
extern BOOL g_AbortFlag;
extern UINT64 g_countNode;
extern char g_AiMsg[128];
extern char g_PVLineMsg[256];
extern SetMessageToGUI g_set_message_funcptr[3];

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
* Name  : NWS_SearchDeep
* Brief : Null Window Search を行い、評価値を基に最善手を取得
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 読む深さ
*         empty     : 空きマス数
*         alpha     : このノードにおける下限値
*         beta      : このノードにおける上限値
*         color     : CPUの色
*         hash      : 置換表の先頭ポインタ
*         pass_cnt  : 今までのパスの数(２カウントで終了とみなす)
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 PVS_SearchDeep(
	UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32 *p_selectivity, PVLINE *pline
);

/***************************************************************************
* Name  : PVS_SearchShallow
* Brief : PV Search を行い、評価値を基に最善手を取得
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         depth     : 読む深さ
*         empty     : 空きマス数
*         alpha     : このノードにおける下限値
*         beta      : このノードにおける上限値
*         color     : CPUの色
*         hash      : 置換表の先頭ポインタ
*         pass_cnt  : 今までのパスの数(２カウントで終了とみなす)
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 PVS_SearchShallow(UINT64 bk, UINT64 wh, INT32 depth, INT32 empty, UINT32 color,
	HashTable *hash, INT32 alpha, INT32 beta, UINT32 passed, INT32 *p_selectivity);

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

BOOL search_SC_PVS(UINT64 bk, UINT64 wh, INT32 empty,
	volatile INT32 *alpha, volatile INT32 *beta, INT32 *score);
void CreatePVLineStr(PVLINE *pline, INT32 empty, INT32 score);

INT32 ProbCutOff(
	UINT64     bk,
	UINT64     wh,
	INT32      empty,
	INT32      depth,
	UINT32     color,
	INT32      alpha,
	INT32      beta,
	UINT32     passed,
	INT32     *score,
	INT32     *serchDepth
);