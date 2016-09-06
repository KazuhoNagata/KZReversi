/***************************************************************************
* Name  : book.cpp
* Brief : 局面と定石データを照らし合わせて着手
* Date  : 2016/02/01
****************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bit64.h"
#include "board.h"
#include "rev.h"
#include "move.h"
#include "eval.h"
#include "board.h"
#include "book.h"
#include "fio.h"
#include "mt.h"

INT32 max_change_num[2];
BOOL g_book_done;

/***************************************************************************
*
* Global
*
****************************************************************************/
/* 指し手の回転・対称変換フラグ */
int TRANCE_MOVE;


BooksNode *g_bookTreeRoot;
BooksNode *g_bestNode;
INT32 g_book_node_count;
/***************************************************************************
*
* ProtoType(private)
*
****************************************************************************/
INT32 SearchBooks(BooksNode *book_root, UINT64 bk, UINT64 wh,
	UINT32 color, UINT32 change, INT32 turn);
BooksNode *SearchBookInfo(BooksNode *book_header, BooksNode *before_book_header,
	UINT64 bk, UINT64 wh, INT32 turn);
INT32 book_alphabeta(BooksNode *header, UINT32 depth, INT32 alpha, INT32 beta,
	UINT32 color, UINT32 change, BooksNode **outBestNode);
VOID SortBookNode(BooksNode *best_node[], INT32 e_list[], INT32 cnt);
INT32 SelectNode(INT32 e_list[], INT32 cnt, UINT32 change, INT32 turn);



/***************************************************************************
* Name  : GetMoveFromBooks
* Brief : 定石からCPUの着手を決定する
* Return: 着手可能位置のビット列
****************************************************************************/
UINT64 GetMoveFromBooks(UINT64 bk, UINT64 wh, UINT32 color, UINT32 change, INT32 turn)
{
	INT64 move;
	if (turn == 0 && bk == BK_FIRST && wh == WH_FIRST)
	{
		UINT64 first_move_list[] = { c4, d3, e6, f5 };
		// 一手目の着手はどこに着手しても同じなのでランダムとする
		int rand = genrand_int32() % 4;
		return first_move_list[rand];

	}
	else
	{
		move = SearchBooks(g_bookTreeRoot, bk, wh, color, change, turn);
	}

	if (move == MOVE_NONE)
	{
		return move;
	}

	return 1ULL << move;

}



/***************************************************************************
* Name  : SearchBooks
* Brief : 定石やからCPUの着手を決定する
* Return: 着手可能位置のビット列
****************************************************************************/
INT32 SearchBooks(BooksNode *book_root, UINT64 bk, UINT64 wh,
	UINT32 color, UINT32 change, INT32 turn)
{
	INT32 move = MOVE_NONE;
	ULONG eval = 0;
	BooksNode *book_header;

	/* 局面から該当の定石を探す */
	book_header = SearchBookInfo(book_root, NULL, bk, wh, turn);

	if (book_header != NULL)
	{
		/* 評価値により次の手を定石から選ぶ */
		eval = book_alphabeta(book_header, 0, NEGAMIN, NEGAMAX, color, change, &g_bestNode);
		book_header = g_bestNode;
		g_evaluation = eval;

		/* 指し手の対称回転変換の場合分け */
		switch (TRANCE_MOVE)
		{
		case 0:
			move = book_header->move;
			break;
		case 1:
		{
			UINT64 t_move = rotate_90(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 2:
		{
			UINT64 t_move = rotate_180(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 3:
		{
			UINT64 t_move = rotate_270(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 4:
		{
			UINT64 t_move = symmetry_x(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 5:
		{
			UINT64 t_move = symmetry_y(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 6:
		{
			UINT64 t_move = symmetry_b(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		case 7:
		{
			UINT64 t_move = symmetry_w(1ULL << book_header->move);
			move = CountBit((t_move & (-(INT64)t_move)) - 1);
		}
		break;
		default:
			move = book_header->move;
			break;
		}
	}

	return move;
}



/***************************************************************************
* Name  : SearchBookInfo
* Brief : 定石からCPUの着手を決定する
* Return: 着手可能位置のビット列
****************************************************************************/
BooksNode *SearchBookInfo(BooksNode *book_header, BooksNode *before_book_header,
	UINT64 bk, UINT64 wh, INT32 turn)
{
	/* 葉ノードまで検索して見つからない場合 */
	if (book_header == NULL)
	{
		return NULL;
	}
	if (book_header->depth > turn)
	{
		return NULL;
	}
	if (book_header->depth == turn)
	{
		do
		{
			/* 該当の定石を発見(回転・対称も考える) */
			if (book_header->bk == bk && book_header->wh == wh)
			{
				/* 指し手の回転・対称変換なし */
				TRANCE_MOVE = 0;
				return book_header;
			}
			/* 90度の回転形 */
			if (book_header->bk == rotate_90(bk) && book_header->wh == rotate_90(wh))
			{
				TRANCE_MOVE = 1;
				return book_header;
			}
			/* 180度の回転形 */
			if (book_header->bk == rotate_180(bk) && book_header->wh == rotate_180(wh))
			{
				TRANCE_MOVE = 2;
				return book_header;
			}
			/* 270度の回転形 */
			if (book_header->bk == rotate_270(bk) && book_header->wh == rotate_270(wh))
			{
				TRANCE_MOVE = 3;
				return book_header;
			}
			/* X軸の対称形 */
			if (book_header->bk == symmetry_x(bk) && book_header->wh == symmetry_x(wh))
			{
				TRANCE_MOVE = 4;
				return book_header;
			}
			/* Y軸の対称形 */
			if (book_header->bk == symmetry_y(bk) && book_header->wh == symmetry_y(wh))
			{
				TRANCE_MOVE = 5;
				return book_header;
			}
			/* ブラックラインの対称形 */
			if (book_header->bk == symmetry_b(bk) && book_header->wh == symmetry_b(wh))
			{
				TRANCE_MOVE = 6;
				return book_header;
			}
			/* ホワイトラインの対称形 */
			if (book_header->bk == symmetry_w(bk) && book_header->wh == symmetry_w(wh))
			{
				TRANCE_MOVE = 7;
				return book_header;
			}
			book_header = book_header->next;
		} while (book_header != NULL);

		return NULL;
	}

	BooksNode *ret;
	if ((ret = SearchBookInfo(book_header->child, book_header, bk, wh, turn)) != NULL)
	{
		return ret;
	}

	book_header = book_header->next;
	while (book_header != NULL)
	{
		/* 該当局面を取得 */
		if ((ret = SearchBookInfo(book_header->child, book_header, bk, wh, turn)) != NULL)
		{
			return ret;
		}
		book_header = book_header->next;
	}
	return NULL;
}



/***************************************************************************
* Name  : book_alphabeta
* Brief : 定石の候補手のうち、後々に評価値が最も高くなるものを算出する
* Return: 定石の評価値
****************************************************************************/
INT32 book_alphabeta(BooksNode *header, UINT32 depth, INT32 alpha, INT32 beta,
	UINT32 color, UINT32 change, BooksNode **outBestNode)
{
	if (header->child == NULL)
	{
		/* 葉ノード(読みの限界値のノード)の場合は評価値を算出 */
		if (color == WHITE) return -header->eval;
		else return header->eval;
	}

	int eval;
	int max;
	BooksNode *iter;

	max = NEGAMIN;
	for (iter = header; alpha < beta && iter != NULL; iter = iter->next)
	{
		/* nextノードの子ノードがNULLの場合があるのでチェック */
		if (iter->child == NULL)
		{
			if (color == WHITE) eval = -iter->eval;
			else eval = iter->eval;
		}
		else
		{
			/* go to child node  */
			eval = -book_alphabeta(iter->child, depth + 1, alpha, beta, color ^ 1, change, outBestNode);
		}

		// βカット
		if (beta <= eval)
		{
			return eval;
		}

		/* 今までより良い局面が見つかれば最善手の更新 */
		if (eval > max)
		{
			max = eval;
			if (depth == 0) *outBestNode = iter;
			if (max > alpha)
			{
				alpha = max;
			}
		}
	}

	return max;
}



/***************************************************************************
* Name  : SortBookNode
* Brief : 定石の候補手のうち、評価値の高い順にソート
****************************************************************************/
void SortBookNode(BooksNode *best_node[], int e_list[], int cnt)
{
	int i = 0;
	int h = cnt * 10 / 13;
	int swaps;
	BooksNode *temp;
	int int_temp;
	if (cnt == 1){ return; }
	while (1)
	{
		swaps = 0;
		for (i = 0; i + h < cnt; i++)
		{
			if (e_list[i] < e_list[i + h])
			{
				temp = best_node[i];
				best_node[i] = best_node[i + h];
				best_node[i + h] = temp;
				int_temp = e_list[i];
				e_list[i] = e_list[i + h];
				e_list[i + h] = int_temp;
				swaps++;
			}
		}
		if (h == 1)
		{
			if (swaps == 0)
			{
				break;
			}
		}
		else
		{
			h = h * 10 / 13;
		}
	}
}



/***************************************************************************
* Name  : SelectNode
* Brief : 定石の変化度によって候補手を決定する
* Return: 定石番号
****************************************************************************/
INT32 SelectNode(int e_list[], int cnt, UINT32 change, INT32 turn)
{
	int ret;
	srand((UINT32)time(NULL));
	if (change == NOT_CHANGE)
	{
		int count;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			if (e_list[count] < max)
			{
				break;
			}
		}
		ret = rand() % count;
	}
	/* 次善手かつ-2以上まで許可 */
	else if (change == CHANGE_LITTLE)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* 一度次善手を選んでいる、
			または次善手が今の評価値より2000以上低いときはbreak */
			if (max_change_num[turn] || max - e_list[count] >= 20000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag)
				{
					break;
				}
				flag++;
			}

		}
		ret = rand() % count;
		/* 次善手を選んだ */
		if (e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else if (change == CHANGE_MIDDLE)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* 一度次善手を選んでいる、
			または次善手が今の評価値より2000以上低いときはbreak */
			if (max_change_num[turn] > 1 || max - e_list[count] >= 40000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag)
				{
					break;
				}
				flag++;
			}

		}
		ret = rand() % count;
		/* 次善手を選んだ */
		if (e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else if (change == CHANGE_ROUGH)
	{
		int count;
		int flag = 0;
		int max = e_list[0];
		for (count = 1; count < cnt; count++)
		{
			/* 一度次善手を選んでいる、
			または次善手が今の評価値より6000以上低いときはbreak */
			if (max_change_num[turn] > 2 || max - e_list[count] >= 60000)
			{
				break;
			}
			if (e_list[count] < max)
			{
				if (flag)
				{
					break;
				}
				flag++;
			}

		}
		ret = rand() % count;
		/* 次善手を選んだ */
		if (e_list[ret] != max)
		{
			max_change_num[turn]++;
		}
	}
	else
	{
		// 完全ランダム
		ret = rand() % cnt;
	}

	return ret;
}



/* ノードを接続 */
void AppendNew(BooksNode *parent, BooksNode *node)
{
	//node->book_name = name;
	//node->move = move;
	node->child = NULL;
	node->next = NULL;
	if (parent == NULL)
	{
		return;
	}
	if (parent->child == NULL)
	{
		parent->child = node;
	}
	else
	{
		while (parent->next != NULL)
		{
			parent = parent->next;
		}
		parent->next = node;
	}
}



/* ノードを接続 */
void Append(BooksNode *parent, BooksNode *node)
{
	if (parent == NULL)
	{
		return;
	}
	if (parent->child == NULL)
	{
		parent->child = node;
	}
	else
	{
		while (parent->next != NULL)
		{
			parent = parent->next;
		}
		parent->next = node;
	}
}



BooksNode *SearchChild(BooksNode *head, int move)
{
	if (head->child == NULL)
	{
		return NULL;
	}
	head = head->child;
	while (head != NULL)
	{
		if (head->move == move)
		{
			return head;
		}
		head = head->next;
	}

	return NULL;
}



void bookline_strtok(char *line_data, char* dest_data, INT32 *eval)
{
	int cnt;
	for (cnt = 0; line_data[cnt] != ';'; cnt++);

	memcpy_s(dest_data, 128, line_data, cnt);
	dest_data[cnt] = '\0';
	*eval = (INT32)(atof(&line_data[cnt + 1]) * EVAL_ONE_STONE);
}



BooksNode *StructionBookTree(BooksNode *head, INT32 *new_eval_p, char *line_data_p, char** next, INT32 depth)
{
	char *decode_sep;
	char move;
	INT32 current_eval;
	UINT64 rev;

	// 行の終わりチェック
	if (line_data_p[depth] == '\0')
	{
		/* ラインが終了したので次ラインを分離 */
		decode_sep = strtok_s(NULL, "\n", next);
		if (decode_sep == NULL)
		{
			// end...
			return NULL;
		}
		// ラインデータと評価値を分離
		bookline_strtok(decode_sep, line_data_p, &current_eval);
		*new_eval_p = current_eval;

		return head;
	}
	else
	{

		// ラインから手を取得
		move = (line_data_p[depth] - 'a') * 8 + line_data_p[depth + 1] - '1';
		// ノードを作成して登録
		BooksNode *child_node = (BooksNode *)malloc(sizeof(BooksNode));
		child_node->depth = depth / 2;
		child_node->eval = *new_eval_p;
		child_node->move = move;
		if (child_node->depth % 2)
		{
			// 黒番
			rev = GetRev[head->move](head->bk, head->wh);
			child_node->bk = head->bk ^ (rev | (1ULL << head->move));
			child_node->wh = head->wh ^ rev;
		}
		else
		{
			// 白番
			rev = GetRev[head->move](head->wh, head->bk);
			child_node->wh = head->wh ^ (rev | (1ULL << head->move));
			child_node->bk = head->bk ^ rev;
		}
		// ノードを登録
		AppendNew(head, child_node);

		/* first entry, start child node area */

		BooksNode *before;
		if (head->child != NULL)
		{
			before = StructionBookTree(head->child, new_eval_p, line_data_p, next, depth + 2);
		}

		if (before == NULL)
		{
			// file end sequence
			return before;
		}

		// check length
		if (depth > strlen(line_data_p)) return head;

		/* next entry, start next node area */
		// ラインから手を取得
		move = (line_data_p[depth - 2] - 'a') * 8 + line_data_p[depth - 1] - '1';
		// ノードを作成して登録
		BooksNode *next_node = (BooksNode *)malloc(sizeof(BooksNode));
		next_node->depth = (depth - 2) / 2;
		next_node->eval = *new_eval_p;
		next_node->move = move;
		if (next_node->depth % 2)
		{
			// 白番
			rev = GetRev[head->move](head->wh, head->bk);
			next_node->wh = head->wh ^ (rev | (1ULL << move));
			next_node->bk = head->bk ^ rev;
		}
		else
		{
			// 黒番
			rev = GetRev[head->move](head->bk, head->wh);
			next_node->bk = before->bk ^ (rev | (1ULL << move));
			next_node->wh = before->wh ^ rev;
		}
		// ノードを登録
		AppendNew(head, next_node);
		if (head->next != NULL)
		{
			head = StructionBookTree(head->next, new_eval_p, line_data_p, next, depth);
		}

		return head;
	}

}

/***************************************************************************
* Name  : OpenBook
* Brief : 定石データを開く
* Return: TRUE/FALSE
****************************************************************************/
BOOL OpenBook(char *filename)
{
	UCHAR* decodeData;
	char *decode_sep, *next_line;
	INT64 decodeDataLen;
	BooksNode *root;
	char line_data[128];
	INT32 eval;

	// ファイルからデコード
	decodeData = DecodeBookData(&decodeDataLen, filename);
	if (decodeDataLen == -1)
	{
		return FALSE;
	}

	// root設定
	root = (BooksNode *)malloc(sizeof(BooksNode));
	root->bk = BK_FIRST;
	root->wh = WH_FIRST;
	root->eval = 0;
	root->depth = 0;
	root->move = 19; /* C4 */
	root->child = NULL;
	root->next = NULL;

	decode_sep = strtok_s(decodeData, "\n", &next_line);
	if (decode_sep == NULL)
	{
		// end...
		return FALSE;
	}
	// ラインデータと評価値を分離
	bookline_strtok(decode_sep, line_data, &eval);
	// 深さ優先で木構造を構築
	StructionBookTree(root, &eval, line_data, &next_line, 2);

	g_bookTreeRoot = root;

	free(decodeData);
	if (g_bookTreeRoot->child == NULL)
	{
		// メモリがない
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************
* Name  : BookFree
* Brief : 定石データのためのメモリを解放
****************************************************************************/
void BookFree(BooksNode *head)
{
	// 葉ノード？
	if (head->next == NULL && head->child == NULL)
	{
		free(head);
		return;
	}

	if (head->child)
	{
		BookFree(head->child);
	}
	if (head->next)
	{
		BookFree(head->next);
	}

	free(head);

	return;
}