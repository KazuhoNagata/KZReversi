/***************************************************************************
* Name  : ai_core.cpp
* Brief : DLLエクスポート関数関連
* Date  : 2016/02/01
****************************************************************************/

#include "stdafx.h"
#include "board.h"
#include "move.h"
#include "book.h"
#include "eval.h"
#include "hash.h"
#include "mt.h"
#include "bit64.h"
#include "cpu.h"
#include "rev.h"

#define KZ_EXPORT __declspec(dllexport)

KZ_EXPORT void KZ_ReleaseBook();

BOOL m_BookFlag;
int g_func_count = 0;

/***************************************************************************
* Name  : KZ_LibInit
* Brief : 初期化処理を行う
* Return: TRUE/FALSE
****************************************************************************/
KZ_EXPORT BOOL KZ_LibInit()
{
	BOOL result;

	// DLLのロード
	result = AlocMobilityFunc();

	if (result == TRUE)
	{
		// 定石データと評価テーブルのロード
		result = LoadData();
	}

	/* pos番号-->指し手文字列変換テーブル */
	char cordinate[6];
	/* 指し手の座標表記変換用 */
	for (int i = 0; i < 64; i++)
	{
		sprintf_s(cordinate, 4, "%c%d", (i / 8) + 'a', (i % 8) + 1);
		strcpy_s(g_cordinates_table[i], sizeof(g_cordinates_table[i]), cordinate);
	}

	edge_stability_init();

	init_genrand((unsigned long)time(NULL));
	//InitHashBoard();

	return result;
}

/***************************************************************************
* Name  : KZ_EnumGetCpuMove
* Brief : 着手可能手を列挙する
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetEnumMove(UINT64 bk_p, UINT64 wh_p, UINT32 *p_count_p)
{
	return CreateMoves(bk_p, wh_p, p_count_p);
}

/***************************************************************************
* Name  : KZ_EnumGetCpuMove
* Brief : 変化する箇所を計算し、ビット列にして返却する
* Return: 変化する箇所のビット列
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetBoardChangeInfo(UINT64 bk, UINT64 wh, INT32 move)
{
	return GetRev[move](bk, wh);
}

/***************************************************************************
* Name  : KZ_GetCpuMove
* Brief : 定石や評価値からCPUの着手を計算する
* Args  : bk 黒の盤面情報
*         wh 白の盤面情報
*         cpuConfig CPU設定クラス
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetCpuMove(UINT64 bk, UINT64 wh, CPUCONFIG *cpuConfig)
{
	UINT64 move = MOVE_NONE;
	UINT32 emptyNum;
	UINT32 turn;

	// カウントリセット(カウントリセットは以降の処理との順番入れ替え×)
	g_countNode = 0;
	// CPUメッセージ初期化
	g_set_message_funcptr[0](" ");
	g_set_message_funcptr[1](" ");

	emptyNum = CountBit(~(bk | wh));
	turn = 60 - emptyNum;

	if (emptyNum == 0)
	{
		return MOVE_PASS;
	}

	if (cpuConfig->bookFlag)
	{
		// 定石データから着手
		m_BookFlag = TRUE;
		move = GetMoveFromBooks(bk, wh, cpuConfig->color,
			cpuConfig->bookVariability, turn);
	}

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// 定石に該当しない局面の場合
	if (move == MOVE_NONE)
	{
		g_book_done = FALSE;
		if (cpuConfig->color == BLACK)
		{
			// 探索開始
			move = GetMoveFromAI(bk, wh, emptyNum, cpuConfig);
		}
		else
		{
			// 探索開始
			move = GetMoveFromAI(wh, bk, emptyNum, cpuConfig);
		}
	}
	else
	{
		g_book_done = TRUE;
	}

	return move;
}

/***************************************************************************
* Name  : KZ_GetLastEvaluation
* Brief : 直前のCPUの着手に対応する評価値を取得する
* Return: CPUの算出した直近評価値
****************************************************************************/
KZ_EXPORT INT32 KZ_GetLastEvaluation()
{
	return g_evaluation;
}

/***************************************************************************
* Name  : KZ_GetIsUseBook
* Brief : 直前のCPUの着手が定石手かどうかを取得する
* Return: CPUの算出した直近評価値
****************************************************************************/
KZ_EXPORT BOOL KZ_GetIsUseBook()
{
	return g_book_done;
}


/***************************************************************************
* Name  : KZ_GetCountNode
* Brief : 探索済みのノード数を取得する
* Return: CPUが探索したノード数
****************************************************************************/
KZ_EXPORT UINT64 KZ_GetCountNode()
{
	return g_countNode;
}

/***************************************************************************
* Name  : KZ_SendAbort
* Brief : AIスレッドに中断命令を送信
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT void KZ_SendAbort()
{
	SetAbortFlag();
}

/***************************************************************************
* Name  : KZ_SendAbort
* Brief : AIスレッドに中断命令を送信
* Return: 着手可能位置のビット列
****************************************************************************/
KZ_EXPORT int KZ_GetIsAbort()
{
	return g_AbortFlag;
}

/***************************************************************************
* Name  : KZ_CountBit
* Brief : １が立っているビット数を数える
* Args  : bit １が立っているビットを数える対象のビット列
* Return: １が立っているビット数
****************************************************************************/
KZ_EXPORT UINT32 KZ_CountBit(UINT64 bit)
{
	return CountBit(bit);
}

/***************************************************************************
* Name  : KZ_EntryFunction
* Brief : AIのメッセージを設定するデリゲートをC側に渡す
* Args  : デリゲートのポインタ
****************************************************************************/
KZ_EXPORT void KZ_EntryFunction(SetMessageToGUI ptr)
{
	g_set_message_funcptr[g_func_count] = ptr;
	g_func_count++;
}

/***************************************************************************
* Name  : KZ_ReleaseHash
* Brief : ハッシュテーブルを解放する
****************************************************************************/
KZ_EXPORT void KZ_ReleaseHash()
{
	if (g_hash != NULL)
	{
		HashClear(g_hash);
		HashDelete(g_hash);
		g_hash = NULL;
	}
}

/***************************************************************************
* Name  : KZ_ReleaseBook
* Brief : 定石データを解放する
****************************************************************************/
KZ_EXPORT void KZ_ReleaseBook()
{
	if (g_bookTreeRoot != NULL)
	{
		BookFree(g_bookTreeRoot);
		g_bookTreeRoot = NULL;
	}
}