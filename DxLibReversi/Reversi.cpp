//#
#include <iterator>
#include <iostream>
#include <string>

#include "DxLib.h"

#include "DataSet.h"
#include "Reversi.h"
#include "ActionBehavior.h"

#ifdef _UNICODE
typedef std::wstringstream tstringstream;
typedef std::wstring tstring;
#else
typedef std::stringstream tstringstream;
typedef std::string tstring;
#endif

using namespace Resolution;

// 初期化
const XMFLOAT2 Reversi::gridAnchor = XMFLOAT2(Resolution::WND_WIDTH * 0.35f, Resolution::WND_HEIGHT * 0.2f);
Reversi::Reversi()
:
turn(0),
memory(),
grids(),
pieceCount()
{
	turn = 0;

	int center = grid / 2;	// マスの中央

	// マスの初期状態を作成
	PutPiece(center - 1, center - 1, (int)pieceColor::Black);
	PutPiece(center - 0, center - 0, (int)pieceColor::Black);
	PutPiece(center - 0, center - 1, (int)pieceColor::White);
	PutPiece(center - 1, center - 0, (int)pieceColor::White);
}

// デストラクタ
Reversi::~Reversi() = default;

// アップデート
void Reversi::Update()
{

}

// 石を置く
bool Reversi::PutPiece(int x, int y, int piece)
{
	// 座標の値が不正
	if (x > std::size(grids) || y > std::size(grids[0]))
	{
		return false;
	}
	// 座標内の要素が不正
	if (grids[x][y] > (int)pieceColor::MAX)
	{
		return false;
	}

	pieceCount[(int)grids[x][y]]--;	// すでに置かれていた石の数を減らす
	pieceCount[piece]++;	// 新しく置かれた石の数を増やす

	grids[x][y] = piece;	// 石を設置する

	return true;
}

// 反転処理
bool Reversi::Inversion(int x, int y, int piece)
{
	bool isSuccess = PutPiece(x, y, piece);	// 起点となる位置に石を置く

	// 起点の位置に石を置くことに失敗したら戻る
	if (!isSuccess) 
	{
		return false;
	}

	for (int g = 0; g < 8; g++)
	{

		std::vector<DirectX::XMINT2> inverGrids;	// 反転させるグリッド
		int _x = x + conn_c[g];			// 列隣方向
		int _y = y + conn_r[g];			// 行隣方向

		// マス範囲外は調べない
		if (_x < 0 || _x >= grid || _y < 0 || _y >= grid)
		{
			continue;
		}
		// 隣接マスが自分の色
		if (grids[_x][_y] == piece)
		{
			continue;
		}
		// 隣接マスに何もない
		if (grids[_x][_y] == (int)pieceColor::None)
		{
			continue;
		}

		while (true)
		{
			// マス範囲外
			if (_x < 0 || _x >= grid || _y < 0 || _y >= grid)
			{
				break;
			}
			// マスに何もない
			if (grids[_x][_y] == (int)pieceColor::None)
			{
				break;
			}
			// 同色
			if (grids[_x][_y] == piece)
			{
				break;
			}

			// 隣接マスに違う色の石があったら記録
			inverGrids.push_back(DirectX::XMINT2{ _x, _y });

			_x += conn_c[g];	// 連鎖回数分ずらす
			_y += conn_r[g];	// 連鎖回数分ずらす
		}

		// マス範囲外
		if (_x < 0 || _x >= grid || _y < 0 || _y >= grid)
		{
			continue;
		}
		
		// 止まったマスが自分の色以外
		if (grids[_x][_y] != piece)
		{
			continue;
		}

		// 記録したマスの石を自分のものにする
		for (auto& grid : inverGrids)
		{
			PutPiece(grid.x, grid.y, piece);
		}
	}
	return true;
}

// マスにおけるか
bool Reversi::UsePut(int x, int y, int piece)
{
	// マスにすでに石が置かれている
	if (grids[x][y] != (int)pieceColor::None)
	{
		return false;
	}

	for (int g = 0; g < 8; g++)
	{

		std::vector<DirectX::XMINT2> inverGrids;	// 反転させるグリッド
		int _x = x + conn_c[g];			// 列隣方向
		int _y = y + conn_r[g];			// 行隣方向

		// 隣接マスが範囲外
		if (_x < 0 || _x >= grid || _y < 0 || _y >= grid)
		{
			continue;
		}
		// 隣接マスに何もない
		if (grids[_x][_y] == (int)pieceColor::None)
		{
			continue;
		}
		// 隣接マスが自分の色
		if (grids[_x][_y] == piece)
		{
			continue;
		}

		while (true)
		{
			// 隣接マスが範囲外
			if (_x < 0 || _x >= grid || _y < 0 || _y >= grid)
			{
				break;
			}
			// 隣接マスに何もない
			if (grids[_x][_y] == (int)pieceColor::None)
			{
				break;
			}
			// 隣接マスが自分の色
			if (grids[_x][_y] == piece)
			{
				break;
			}

			_x += conn_c[g];	// 連鎖回数分ずらす
			_y += conn_r[g];	// 連鎖回数分ずらす
		}

		// マス範囲外
		if (_x < 0 || _x >= grid || _y < 0 || _y >= grid)
		{
			continue;
		}

		// 止まったマスが自分の色以外
		if (grids[_x][_y] != piece)
		{
			continue;
		}

		// おける場所
		return true;
	}

	// どこにも置けない
	return false;
}

// 石の数を数える
void Reversi::CountPiece()
{

	for (int r = 0; r < grid; r++)
	{
		for (int c = 0; c < grid; c++)
		{
			if (grids[r][c] == (int)pieceColor::Black)
			{
				pieceCount[(int)pieceColor::Black]++;
			}
			else if (grids[r][c] == (int)pieceColor::White)
			{
				pieceCount[(int)pieceColor::White]++;
			}
			else
			{
				pieceCount[(int)pieceColor::None]++;
			}
		}
	}
}

// ゲーム終了条件を満たしているか
int Reversi::IsGameFinished()
{
	int black = (int)pieceColor::Black;
	int white = (int)pieceColor::White;
	bool canB = CanMove(black);
	bool canW = CanMove(white);
	bool isGameFinish = (!canB && !canW);

	return isGameFinish;
}

// 行動できるか
int Reversi::CanMove(int piece)
{
	bool canMove = false;

	for (int c = 0; c < std::size(grids); c++)
	{
		for (int r = 0; r < std::size(grids[0]); r++)
		{
			bool canPut = UsePut(c, r, piece);
			canMove = canMove || canPut;	// 置けるマスか
		}
	}


	return canMove;
}

// ゲームの勝敗
int Reversi::GetResult(int piece)
{
	int myGetGrid = pieceCount[piece];
	int othersGetGrid = (piece == (int)pieceColor::Black) ? pieceCount[(int)pieceColor::Black] : pieceCount[(int)pieceColor::White];
	
	if (myGetGrid > othersGetGrid)
	{
		return (int)result::Win;
	}
	else if (myGetGrid < othersGetGrid)
	{
		return (int)result::Lose;
	}
	else
	{
		return (int)result::Draw;
	}
}

// 描画：グリッド
void Reversi::ViewGrid()
{
	// グリッドを描画
	const unsigned int edgeColor = GetColor(190, 170, 250);	// 淵の色
	constexpr int gridThick = 3;	// グリッドの太さ

	for (int c = 0; c < std::size(grids[0]); c++)
	{
		for (int r = 0; r < std::size(grids); r++)
		{
			for (int t = 0; t < gridThick; t++)
			{
				int baseX = gridAnchor.x + gridPadding * c + gridSize * c;
				int baseY = gridAnchor.y + gridPadding * r + gridSize * r;

				int x1 = baseX - t;
				int x2 = baseX + gridSize + t;
				int y1 = baseY - t;
				int y2 = baseY + gridSize + t;

				DrawBox(x1, y1, x2, y2, edgeColor, FALSE);
			}
		}
	}
}

// 描画：状態
void Reversi::ViewState(int localColor)
{
	// マスの状態を描画
	const unsigned int pushColor = GetColor(180, 180, 200);	// 設置できるマスの色

	for (int c = 0; c < std::size(grids); c++)
	{
		for (int r = 0; r < std::size(grids[0]); r++)
		{
			// すでに置かれているマスは即コンテニュー
			if (grids[c][r] == (int)pieceColor::None)
			{
				continue;
			}

			int x1 = (gridAnchor.x + gridPadding * c + gridSize * c) + stateVert;
			int y1 = (gridAnchor.y + gridPadding * r + gridSize * r) + stateVert;

			// 対応する色の石を描画
			if (grids[c][r] == (int)pieceColor::Black)
			{
				DrawCircle(x1, y1, stateSize, Color::black, TRUE);
			}
			else if (grids[c][r] == (int)pieceColor::White)
			{
				DrawCircle(x1, y1, stateSize, Color::white, TRUE);
			}

			continue;
		}
	}
}

// 描画：ガイド
// 石を置ける場所をガイドする
void Reversi::ViewGuide(int localColor)
{
	// マスの状態を描画
	const unsigned int pushColor = GetColor(180, 180, 200);	// 設置できるマスの色
	constexpr float guideStateSize = gridSize * 0.2f;	// ガイド用ステートの大きさ

	for (int c = 0; c < std::size(grids); c++)
	{
		for (int r = 0; r < std::size(grids[0]); r++)
		{
			bool _usePut = UsePut(c, r, localColor);	// 置けるマスか

			// すでに置かれているか置けるマスではない場合は即時コンテニュー
			if (grids[c][r] != (int)pieceColor::None || !_usePut)
			{
				continue;
			}

			int x1 = (gridAnchor.x + gridPadding * c + gridSize * c) + stateVert;
			int y1 = (gridAnchor.y + gridPadding * r + gridSize * r) + stateVert;

			DrawCircle(x1, y1, guideStateSize, pushColor, TRUE);
		}
	}
}

// 描画：UI
// 文字は中央ぞろえにしたいかもなー。
void Reversi::ViewUI(int localColor, int& font)
{

	// UIを描画
	const bool localIsBlack = (localColor == (int)pieceColor::Black) ? true : false;	// ローカルが黒色の石か
	constexpr XMFLOAT2 leftMargin = XMFLOAT2(WND_WIDTH * 0.1f, WND_HEIGHT * 0.2f);	// 左に置くときの左からの余白
	constexpr XMFLOAT2 rightMargin = XMFLOAT2(WND_WIDTH * 0.8f, WND_HEIGHT * 0.2f);	// 右に置くときの左からの余白
	const XMFLOAT2 blackAnchor = (localIsBlack) ? leftMargin  : rightMargin;	// 黒の座標アンカー
	const XMFLOAT2 whiteAnchor = (localIsBlack) ? rightMargin : leftMargin;		// 白の座標アンカー
	const XMFLOAT2 localAnchor = (localIsBlack) ? blackAnchor : whiteAnchor;	// ローカルの座標アンカー
	const XMFLOAT2 peerAnchor  = (localIsBlack) ? whiteAnchor : blackAnchor;	// 相手の座標アンカー

	constexpr XMFLOAT2 score = XMFLOAT2(WND_WIDTH * 0.0f, WND_HEIGHT * 0.1f);	// スコアのベース表示位置
	const unsigned int scoreColor = GetColor(50, 50, 50);	// スコアの色

	// ローカルのデータUI
	std::string myColor = (localIsBlack) ? "黒" : "白";
	std::string myGuide = "あなた：" + myColor;
	DrawStringToHandle(localAnchor.x, localAnchor.y, myGuide.c_str(), Color::black, font);

	// 相手のデータUI
	std::string peerColor = (localIsBlack) ? "白" : "黒";
	std::string peerGuide = "相手：" + peerColor;
	DrawStringToHandle(peerAnchor.x, peerAnchor.y, peerGuide.c_str(), Color::black, font);

	// 黒色のデータUI
	std::string blackScore = std::to_string(pieceCount[(int)pieceColor::Black]);
	DrawStringToHandle(score.x + blackAnchor.x, score.y + blackAnchor.y, blackScore.c_str(), scoreColor, font);

	// 白色のデータUI
	std::string whiteScore = std::to_string(pieceCount[(int)pieceColor::White]);
	DrawStringToHandle(score.x + whiteAnchor.x, score.y + whiteAnchor.y, whiteScore.c_str(), scoreColor, font);
}