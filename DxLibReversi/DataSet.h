//#
#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include "DxLib.h"

using namespace DirectX;

namespace Color
{
	static const unsigned int black = GetColor(50, 50, 50);		// 黒
	static const unsigned int white = GetColor(250, 250, 250);	// 白
}

namespace Resolution
{
	static constexpr float WND_WIDTH = 1280.0f;	// 幅の解像度
	static constexpr float WND_HEIGHT = 720.0f;	// 高さの解像度
}

namespace RaruLib
{
	// 0～解像度の座標系をuvに変換する
	static XMFLOAT2 NormalizeToCenter(XMFLOAT2 pos)
	{
		XMFLOAT2 out(0, 0);

		// 範囲外の値を丸める
		float x = std::clamp(pos.x, 0.0f, Resolution::WND_WIDTH);
		float y = std::clamp(pos.y, 0.0f, Resolution::WND_HEIGHT);

		// 0～1に正規化
		x = x / Resolution::WND_WIDTH;
		y = y / Resolution::WND_HEIGHT;

		// y座標は上が-1になるようにする
		y = 1.0f - y;

		// uvの座標系に変換して取得
		out.x = x * 2.0f - 1.0f;
		out.y = y * 2.0f - 1.0f;

		return out;
	}

	// uvの座標系を左上が0,解像度の座標系に変換する
	static XMFLOAT2 ConvertNormalizedCenterToPixel(XMFLOAT2 pos)
	{
		XMFLOAT2 out(0, 0);

		// 範囲外の値を丸める
		float x = std::clamp(pos.x, -1.0f, 1.0f);
		float y = std::clamp(pos.y, -1.0f, 1.0f);

		// UVを0～1に丸める
		x = (x + 1.0f) * 0.5f;
		y = (y + 1.0f) * 0.5f;

		// y座標は上が-1になっているので反転
		y = 1.0f - y;

		// uvの座標系を解像度に変換して取得
		out.x = x * Resolution::WND_WIDTH;
		out.y = y * Resolution::WND_HEIGHT;

		return out;
	}
}