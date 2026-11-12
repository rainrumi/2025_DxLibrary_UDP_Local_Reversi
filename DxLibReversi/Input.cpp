//#
#include "Input.h"

#include "DxLib.h"
#include "Reversi.h"

// 座標に対応するグリッドを取得
XMINT2 Input::GetGrid(XMINT2 pos) const
{
	auto grid = Reversi::grid;
	auto gridPadding = Reversi::gridPadding;
	auto gridSize = Reversi::gridSize;
	auto gridAnchor = Reversi::gridAnchor;
	float offset = gridSize + gridPadding;	// マス一つ分のオフセット

	// グリッド範囲外の座標の時は(-1,-1)を返す
	float maxOffset = offset * grid;
	XMFLOAT2 maxPosition = XMFLOAT2(gridAnchor.x + maxOffset, gridAnchor.y + maxOffset);
	if (pos.x < gridAnchor.x || pos.x >= maxPosition.x || pos.y < gridAnchor.y || pos.y >= maxPosition.y)
	{
		return XMINT2(-1, -1);
	}

	// gridSizeを増やしながら押したグリッドの位置を調べる
	for (int c = 0; c < grid; c++)
	{
		for (int r = 0; r < grid; r++)
		{
			bool isHitX = pos.x >= gridAnchor.x + offset * c && pos.x < gridAnchor.x + offset * (c + 1);
			bool isHitY = pos.y >= gridAnchor.y + offset * r && pos.y < gridAnchor.y + offset * (r + 1);
			if (isHitX && isHitY)
			{
				return XMINT2(c, r);
			}
		}
	}
	return XMINT2(-1, -1);
}

// 指定マウスキーをクリックしたか
bool Input::WasMouseClicked(int key) const
{
	int button = 0;
	XMINT2 pos = XMINT2(0, 0);
	int log;

	if (GetMouseInputLog2(&button, &pos.x, &pos.y, &log, TRUE) == 0)
	{
		// 任意ボタンが押されたり離されたりしていたら描画するかどうかのフラグを立てて、座標も保存する
		if ((button & key) != 0 && log == MOUSE_INPUT_LOG_DOWN)
		{
			return true;
		}
	}

	return false;
}

int Input::ClearMouseClickedLog(int key) const
{
	int result = 0;
	while (result == 0)
	{
		result = GetMouseInputLog2(&key, nullptr, nullptr, nullptr, TRUE);
	}

	return 0;
}