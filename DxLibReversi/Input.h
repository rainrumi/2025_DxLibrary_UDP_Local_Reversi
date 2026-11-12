//#
#pragma once
#include <utility>
#include <DirectXMath.h>
#include "DxLib.h"

using namespace DirectX;

class Input
{
public:
	inline static constexpr int putPiece = MOUSE_INPUT_LEFT;	// 石を置く
public:
	bool WasMouseClicked(int) const;	// 指定マウスキーをクリックしたか
	XMINT2 GetGrid(XMINT2) const;	// 座標に対応するグリッドを取得
	int ClearMouseClickedLog(int) const;	// 指定マウスキーのクリックログを開放する
};