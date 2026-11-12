//#
// 【Strategyパターン [C#][C++]】 https://qiita.com/WestRiver/items/e6c721cd35d5a7183d04

#pragma once
#include <iostream>
#include <DirectXMath.h>

using namespace DirectX;

class Reversi;
class NetworkUDP;
struct inversionData;
//#include "NetworkUDP.h"

enum class NetworkRole;

class WaitingConn;
class Input;
class LocalTurn;
class WaitingResponse;
class PeerTurn;
class GameFinish;
class ErrorState;


// アクションの種類
enum class ActionState
{
	None,
	WaitingConn,
	LocalTurn,
	WaitingResponse,
	PeerTurn,
	GameFinish,
	ErrorState,
	MAX
};


// アクションベース
class ActionBehavior
{
public:
	virtual ~ActionBehavior() = default;
	void Update(Reversi& reversi, NetworkUDP& net,const Input& input,const int& localColor, const int& font)
	{
		this->requestCurrentStateCache = ActionState::None;		// キャッシュをリセットする
		this->UpdateLogic(reversi, net, input, localColor);
		this->Network(net, localColor);
		this->View(font);
	}
	ActionState GetRequestCurrentState() const;
private:
	virtual void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&) = 0;	// 内部処理
	virtual void Network(NetworkUDP&, const int&) = 0;			// 内部処理を基にしたネットワーク通信
	virtual void View(const int&) = 0;							// 内部情報を基にした描画
protected:
	ActionState requestCurrentStateCache = ActionState::None;	// 遷移したいクラスを示すキャッシュ（どうするべき？）
};

// コネクション待機
class WaitingConn :public ActionBehavior
{
private:
	void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&)  override;
	void Network(NetworkUDP&, const int&) override;
	void View(const int&) override;
};

// 自ターン
class LocalTurn :public ActionBehavior
{
private:
	void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&)  override;
	void Network(NetworkUDP&, const int&) override;
	void View(const int&) override;
private:
	bool canMove = false;		// 設置可能か
	bool wasMoved = false;		// 行動したか
	inline static constexpr XMINT2 initHitGrids = XMINT2(-1, -1);
	XMINT2 hitGrids = initHitGrids;	// 選択したグリッド
};

// 応答待機
class WaitingResponse :public ActionBehavior
{
public:
	WaitingResponse();
private:
	void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&)  override;
	void Network(NetworkUDP&, const int&) override;
	void View(const int&) override;

private:
	std::unique_ptr<inversionData> recvData;
};

// 相手ターン
class PeerTurn :public ActionBehavior
{
public:
	PeerTurn();
private:
	void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&)  override;
	void Network(NetworkUDP&, const int&) override;
	void View(const int&) override;

private:
	std::unique_ptr<inversionData> recvData;
};

// ゲーム終了
class GameFinish :public ActionBehavior
{
private:
	void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&)  override;
	void Network(NetworkUDP&, const int&) override;
	void View(const int&) override;
};

// エラー
class ErrorState :public ActionBehavior
{
private:
	void UpdateLogic(Reversi&, const NetworkUDP&, const Input&, const int&)  override;
	void Network(NetworkUDP&, const int&) override;
	void View(const int&) override;
};


/*****************************************************************************************************/
// アクション汎用メソッド
namespace Action
{
	// アクションの変更
	void ChangeAction(std::unique_ptr<ActionBehavior>& originalAction, ActionState state);
}