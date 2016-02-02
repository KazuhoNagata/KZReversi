/***************************************************************************
* Name  : search.h
* Brief : 探索の処理全般を行う
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"

#define KEY_HASH_MACRO(b, w) (UINT32)((b ^ ((w) >> 1)) % g_casheSize)

#define ILLIGAL_ARGUMENT 0x80000001

typedef struct
{
	UINT32 color;				// CPUの色
	UINT32 casheSize;			// 置換表のサイズ
	UINT32 searchDepth;			// 中盤読みの深さ
	UINT32 winLossDepth;		// 勝敗探索を開始する深さ
	UINT32 exactDepth;			// 石差探索を開始する深さ
	BOOL   bookFlag;			// 定石を使用するかどうか
	BOOL   bookVariability;		// 定石の変化度
	BOOL   mpcFlag;				// MPCを使用するかどうか
	BOOL   tableFlag;			// 置換表を使用するかどうか

}CPUCONFIG;

extern INT32 g_limitDepth;
extern UINT64 g_casheSize;

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
* Name  : OrderingAlphaBeta
* Brief : 着手可能手の並び替えを浅い探索によって決定する
* Args  : bk        : 黒のビット列
*         wh        : 白のビット列
*         empty     : 空白マスの数
*         cpuConfig : CPUの設定
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 OrderingAlphaBeta(UINT64 bk, UINT64 wh, UINT32 depth,
	INT32 alpha, INT32 beta, UINT32 color, UINT32 turn, UINT32 pass_cnt);