//#
#pragma once
#include <iostream>
#include <array>
#include <string>
#include "DxLib.h"

class Reversi;
class Input;
class Player;
class NetworkUDP;
class ActionBehavior;
struct AnimationData;

// フォントの種類
enum class FontType
{
	RoundedMplus = 0,
	MAX
};

// アニメーションの種類
enum class AnimType
{
	Sample = 0,
	MAX
};

// メイン
class main
{
public:
	main();
	~main();
	void mainProc();
	void NetworkSet();
private:
	IPDATA GetInputIp(const std::string&);
private:
	std::unique_ptr<Reversi> reversi;		// リバーシ本体
	std::unique_ptr<Input> input;			// 入力感知
	std::unique_ptr<Player> localPlayer;	// 自身のデータ
	std::unique_ptr<Player> peerPlayer;		// 相手のデータ
	std::unique_ptr<NetworkUDP> networkUdp;	// ネットワーク周り
	std::unique_ptr<ActionBehavior> action;	// 現在のアクション

	std::array<int, (int)FontType::MAX> font;					// フォントデータ
	std::array<std::unique_ptr<AnimationData>, (int)AnimType::MAX> animation;	// アニメーションデータ

	//static constexpr std::array<std::string, (int)AnimType::MAX> animationFileName = { "AnimationData" };
};