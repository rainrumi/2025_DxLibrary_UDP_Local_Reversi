//#
#pragma once

#include <cstdint>
#include <vector>
#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

enum class result
{
	Win,
	Lose,
	Draw,
	MAX
};

enum class pieceColor
{
	None = 0,	// 無し
	Black,		// 黒
	White,		// 白
	MAX
};

class Reversi
{
public:
	inline static constexpr int grid = 8;	// マスの数
	static const XMFLOAT2 gridAnchor;		// マスの座標アンカー
	inline static constexpr int gridSize = 50;	// マスのサイズ
	inline static constexpr int gridPadding = 10;	// グリッド間の余白
	inline static constexpr float stateVert = gridSize * 0.5f;	// ステートの頂点の位置
	inline static constexpr float stateSize = gridSize * 0.4f;	// ステートの大きさ
private:
	inline static constexpr int8_t conn_r[8] = { -1,-1,-1,0,0,1,1,1 };	// 隣接列
	inline static constexpr int8_t conn_c[8] = { -1,0,1,-1,1,-1,0,1 };	// 隣接行
private:
	int grids[grid][grid];		// マスのデータ(黒白も含む)
	int turn;					// ターン
	std::vector<int> memory;	// 盤面メモリ
	int pieceCount[(int)pieceColor::MAX];		// 無黒白の数
	
public:
	Reversi();
	~Reversi();
	bool PutPiece(int, int, int);	// 石を置く
	bool Inversion(int, int, int);	// 反転処理
	bool UsePut(int, int, int);		// 置ける場所か
	void CountPiece();				// 石の数を数える
	int IsGameFinished();			// ゲーム終了条件を満たしたか
	int CanMove(int);				// 行動できるか
	int GetResult(int);				// ゲームの勝敗 
	void ViewGrid();				// 描画：グリッド
	void ViewState(int);			// 描画：状態
	void ViewGuide(int);			// 描画：ガイド
	void ViewUI(int, int&);			// 描画：UI
	void Update();
};