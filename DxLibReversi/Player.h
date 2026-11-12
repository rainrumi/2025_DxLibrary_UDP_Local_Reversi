//#
#pragma once

#include <string>



class Player
{
private:
	int color;	// 色
	const std::string* name;	// 名前
public:

	Player(const std::string&);
	int GetMyColor() const;	// 自分の色を返す
	int SetMyColor(int);	// 自分の色をセットする
};