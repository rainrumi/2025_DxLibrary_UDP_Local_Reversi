//#
#include "ActionBehavior.h"

#include "DxLib.h"
#include <DirectXMath.h>
#include <variant>

#include "DataSet.h"
#include "Input.h"
#include "Player.h"
#include "Reversi.h"
#include "NetworkUDP.h"

using namespace DirectX;
using namespace Resolution;

// 遷移したいクラスを返す
ActionState ActionBehavior::GetRequestCurrentState() const
{
	return this->requestCurrentStateCache;
}

/**********************************************************************************************************/
//************ コネクション待機 ************//
//***************内部処理************//
void WaitingConn::UpdateLogic(Reversi& reversi, const NetworkUDP& net, const Input& input, const int& localColor)
{

}
//*************ネットワーク**********//
void WaitingConn::Network(NetworkUDP& net, const int& localColor)
{

}
//***************描画***************//
void WaitingConn::View(const int& font)
{

}

/**********************************************************************************************************/
//************ 応答待機 ************//
WaitingResponse::WaitingResponse()
	:
	recvData(nullptr)
{

}
//***************内部処理************//
void WaitingResponse::UpdateLogic(Reversi& reversi, const NetworkUDP& net, const Input& input, const int& localColor)
{
	if (recvData == nullptr)
	{
		return;
	}
	else
	{
		int a = 0;
	}

	reversi.Inversion(recvData->pos.x, recvData->pos.y, recvData->color); // 受信した相手の石を置く

	recvData = nullptr;	// データを消す

	requestCurrentStateCache = ActionState::PeerTurn;	// アクション切り替え
}

//*************ネットワーク**********//
void WaitingResponse::Network(NetworkUDP& net, const int& localColor)
{
	// 情報が届いていないか確認する
	int result = net.CheckRecvData();
	if (result < 0)
	{
		return;	// 何も届いていない
	}

	if (result == (int)PackType::REQUEST)
	{
		auto getRecvData = net.RecvRequest();	// データを受信する

		// 届いたデータが反転処理のデータの場合（なんかキモイな...）
		if (std::holds_alternative<inversionData>(getRecvData))
		{
			inversionData& _getRecvData = std::get<inversionData>(getRecvData);
			std::unique_ptr<inversionData> _recvData = std::make_unique<inversionData>(_getRecvData.type, _getRecvData.color, _getRecvData.pos);
			recvData = std::move(_recvData);	// データを保持する
		}
		else
		{
			int a = 0;
		}
	}
	else if (result == (int)PackType::ACK)
	{
		net.RecvAck();	// データのリクエスト受理を受信する
	}
	else
	{
		return;	// 異常データは無視
	}

}

//***************描画***************//
void WaitingResponse::View(const int& font)
{
	DrawStringToHandle(WND_WIDTH * 0.5f, WND_HEIGHT * 0.1f, "相手の応答待ち...", Color::black, font);
}

/**********************************************************************************************************/
//************ 自ターン ************//
//***************内部処理************//
void LocalTurn::UpdateLogic(Reversi& reversi, const NetworkUDP& net, const Input& input, const int& localColor)
{
	XMINT2 mousePos = XMINT2(0, 0);

	//メンバ変数の更新
	canMove = reversi.CanMove(localColor);
	hitGrids = initHitGrids;

	// 終了条件を満たしていたらゲームクリア(ここに書かない方がよいかも)
	if (reversi.IsGameFinished())
	{
		requestCurrentStateCache = ActionState::GameFinish;	// アクション切り替え
	}

	// ガイドを描画する
	reversi.ViewGuide(localColor);

	// クリックせずまだ設置もしていなかったら戻る
	if (!input.WasMouseClicked(input.putPiece))
	{
		return;
	}

	if (canMove)
	{
		GetMousePoint(&mousePos.x, &mousePos.y);	// マウスの座標を調べる
		hitGrids = input.GetGrid(mousePos);	// マウスの座標に当てはまるマスを取得

		// 配置可能だが置けない場所ならリターン
		if (!reversi.UsePut(hitGrids.x, hitGrids.y, localColor))
		{
			return;
		}

		wasMoved = true;	// 行動を終えたとして記録する
	}
	else
	{
		wasMoved = true;	// 行動を終えたとして記録する
	}

	// 自身のロールを取得する
	auto role = net.GetRole();

	// ロールに合わせた更新を行う
	if (canMove && role == (int)NetworkRole::SERVER)
	{
		reversi.Inversion(hitGrids.x, hitGrids.y, localColor);	// サーバーは自分が持っているデータで石を置く
	}

	// データを作成したらアクションを切り替えて待機する
	if (role == (int)NetworkRole::SERVER)
	{
		requestCurrentStateCache = ActionState::PeerTurn;	// アクション切り替え
	}
	else if (role == (int)NetworkRole::CLIENT)
	{
		requestCurrentStateCache = ActionState::WaitingResponse;	// アクション切り替え
	}

	return;
}

//*************ネットワーク**********//
void LocalTurn::Network(NetworkUDP& net, const int& localColor)
{
	// まだ行動していないなら処理を行わない
	if (!wasMoved)
	{
		return;
	}

	// 石を置ける場所のデータを作成
	auto pack = net.CreateinversionData(localColor, hitGrids);
	int result = net.SendData(pack);
	int a = 0;
}

//***************描画***************//
void LocalTurn::View(const int& font)
{
	DrawStringToHandle(WND_WIDTH * 0.45f, WND_HEIGHT * 0.1f, "あなたのターン", Color::black, font);

	// 置けるマスがなかったらスキップ
	if (!canMove)
	{
		DrawStringToHandle(WND_WIDTH * 0.5f, WND_HEIGHT * 0.15f, "置ける場所が存在しません。画面クリックでパスします", Color::black, font);
	}
}

/**********************************************************************************************************/
//************ 相手ターン ************//
//***************内部処理************//
PeerTurn::PeerTurn()
	:
	recvData(nullptr)
{

}
void PeerTurn::UpdateLogic(Reversi& reversi, const NetworkUDP& net, const Input& input, const int& localColor)
{
	// 終了条件を満たしていたらゲームクリア(ここに書かない方がよいかも)
	if (reversi.IsGameFinished())
	{
		requestCurrentStateCache = ActionState::GameFinish;	// アクション切り替え
	}

	if (recvData == nullptr)
	{
		return;
	}

	reversi.Inversion(recvData->pos.x, recvData->pos.y, recvData->color); // 受信した相手の石を置く
	recvData = nullptr;	// データを消す

	requestCurrentStateCache = ActionState::LocalTurn;	// アクション切り替え

	return;
}

//*************ネットワーク**********//
void PeerTurn::Network(NetworkUDP& net, const int& localColor)
{
	// 情報が届いていないか確認する
	int result = net.CheckRecvData();
	if (result < 0)
	{
		return;	// 何も届いていない
	}

	if (result == (int)PackType::REQUEST)
	{
		auto getRecvData = net.RecvRequest();
		inversionData& _getRecvData = std::get<inversionData>(getRecvData);	// データを受信する
		std::unique_ptr<inversionData> _recvData = std::make_unique<inversionData>(_getRecvData.type, _getRecvData.color, _getRecvData.pos);
		recvData = std::move(_recvData);	// データを保持する

		// サーバーは常に石の基準となるので自身がサーバーの場合はクライアントに石を置くよう指示する
		if (net.GetRole() == (int)NetworkRole::SERVER)
		{
			auto pack = net.CreateinversionData(recvData->color, recvData->pos);
			net.SendData(pack);
		}
	}
	else if (result == (int)PackType::ACK)
	{
		net.RecvAck();	// データのリクエスト受理を受信する
		//net.RecvRequest();	// データを受信する
	}
	else
	{
		return;	// 異常データは無視
	}
}

//***************描画***************//
void PeerTurn::View(const int& font)
{
	DrawStringToHandle(WND_WIDTH * 0.45f, WND_HEIGHT * 0.1f, "相手のターンが進行中...", Color::black, font);
}

/**********************************************************************************************************/
//************ ゲーム終了 ************//
//***************内部処理************//
void GameFinish::UpdateLogic(Reversi& reversi, const NetworkUDP& net, const Input& input, const int& localColor)
{

}

//*************ネットワーク**********//
void GameFinish::Network(NetworkUDP& net, const int& localColor)
{

}

//***************描画***************//
void GameFinish::View(const int& font)
{
	DrawStringToHandle(WND_WIDTH * 0.5f, WND_HEIGHT * 0.1f, "ゲーム終了！", Color::black, font);
}

/**********************************************************************************************************/
//************ エラー ************//
//***************内部処理************//
void ErrorState::UpdateLogic(Reversi& reversi, const NetworkUDP& net, const Input& input, const int& localColor)
{

}

//*************ネットワーク**********//
void ErrorState::Network(NetworkUDP& net, const int& localColor)
{

}

//***************描画***************//
void ErrorState::View(const int& font)
{

}

/**********************************************************************************************************/
namespace Action
{
	void ChangeAction(std::unique_ptr<ActionBehavior>& originalAction, ActionState state)
	{
		std::unique_ptr<ActionBehavior> newAction = nullptr;

		switch (state)
		{
		case ActionState::None:
		case ActionState::MAX:
			return;
		case ActionState::WaitingConn:
			newAction = std::make_unique<WaitingConn>();
			break;
		case ActionState::LocalTurn:
			newAction = std::make_unique<LocalTurn>();
			break;
		case ActionState::WaitingResponse:
			newAction = std::make_unique<WaitingResponse>();
			break;
		case ActionState::PeerTurn:
			newAction = std::make_unique<PeerTurn>();
			break;
		case ActionState::GameFinish:
			newAction = std::make_unique<GameFinish>();
			break;
		case ActionState::ErrorState:
			newAction = std::make_unique<ErrorState>();
			break;
		}

		if (newAction == nullptr)
		{
			return;
		}

		originalAction = std::move(newAction);	// アクションの所有権を移動させる

		return;
	}
}