#include "Player.h"
#include "Reversi.h"

Player::Player(const std::string& name)
	:
	color((int)pieceColor::None),
	name(&name)
{

}

// 自分の色を返す
int Player::GetMyColor() const
{
	return color;
}

// 自分の色をセットする
int Player::SetMyColor(int newColor)
{
	// すでに色をセットしていたら-1
	if (color != (int)pieceColor::None)
	{
		return -1;
	}
	
	color = newColor;

	return 0;
}