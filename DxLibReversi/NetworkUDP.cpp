//#
#include <chrono>

#include"NetworkUDP.h"

std::atomic<int> NetworkUDP::netid = 0;	// id初期化(一意)

// コンストラクタ
NetworkUDP::NetworkUDP(IPDATA _myip, IPDATA _peerip, int _netUDPSocket, int _role)
	:
	myip(_myip),
	peerip(_peerip),
	netUDPSocket(_netUDPSocket),
	role(_role)
{ }

NetworkUDP::~NetworkUDP() = default;

// inversionDataの作成
requestValue NetworkUDP::CreateinversionData(int color, XMINT2 pos)
{

	return requestValue{ inversionData(PackType::REQUEST, color, pos) };
}

// ネットソケット取得
int NetworkUDP::GetUDPSocket()
{
	return netUDPSocket;
}

// 相手のIP取得
IPDATA NetworkUDP::GetPeerIP()
{
	return peerip;
}

// 自身のIP取得
IPDATA NetworkUDP::GetMyIP()
{
	return myip;
}

// 自身の役割取得
int NetworkUDP::GetRole() const
{
	return role;
}

// データの送信
int NetworkUDP::SendData(requestValue& buffer)
{
	int result = NetWorkSendUDP(GetUDPSocket(), GetPeerIP(), port, &buffer, sizeof(buffer));	// 送信
	if (result < 0)
	{
		return result;
	}
	auto nowTime = std::chrono::high_resolution_clock::now();	// 現在の経過時間を取得
	std::unique_ptr<requestData> newRequestData = std::make_unique <requestData>(nowTime, buffer);	// リクエストリスト用の構造体へ変換

	int id = newRequestData->data.GetID();
	requestLists.emplace(id, std::move(newRequestData));	// リクエストリストに保存
	// ここで送信できたか確認しないのは既に相手から何かしらのデータが送られてきていた場合に対応できないためである
	
	return 0;
}

// データの受信情報確認
int NetworkUDP::CheckRecvData()
{
	if (!CheckNetWorkRecvUDP(GetUDPSocket()))
	{
		return -1;
	}

	requestValue sample = requestValue{ inversionPacket{} };
	int result = NetWorkRecvUDP(GetUDPSocket(), NULL, NULL, &sample, sizeof(sample), TRUE);	// ここではデータをサンプリングするだけ
	//リファクタリング版（これ）だとなぜかここの下で-3が一度返される。多分ActionBehaviorで何かおかしい

	if (result < 0)
	{
		return result;
	}

	std::unique_ptr<requestStruct> getData = std::make_unique <requestStruct>(sample);	// デモデータを作成

	return (int)getData->GetType();	// 自身のデータタイプを返す
}

// データの受信 // データが存在しないとエラーになるのでCheckRecvDataで確認してから入るように
requestValue NetworkUDP::RecvRequest()
{
	requestValue packet = requestValue{ inversionPacket{} };
	int result = NetWorkRecvUDP(GetUDPSocket(), NULL, NULL, &packet, sizeof(packet), FALSE);
	
	// 受信に失敗したら-1,-1データを返す
	if (result < 0)
	{
		int color = GetColor(0, 0, 0);
		return requestValue{ inversionData{ PackType::REQUEST, color, XMINT2(-1, -1) } };
	}

	std::unique_ptr<requestStruct> getData = std::make_unique <requestStruct>(packet);	// デモデータを作成

	// 相手側にackデータを返す
	ackData ack = ackData(getData->GetID());
	result = NetWorkSendUDP(GetUDPSocket(), GetPeerIP(), port, &ack, sizeof(ack));

	// すでに受け取ったデータなら-2,-2データを返す
	if (IsIdInLog(getData->GetID()))
	{
		int color = GetColor(0, 0, 0);
		return requestValue{ inversionData{ PackType::REQUEST, color, XMINT2(-2, -2) } };
	}

	// 初めて受け取ったデータならキューに保存してデータを返す
	// キューに保存
	log.push(getData->GetID());
	// 上限を超過していたら一番古いものを削除
	if (log.size() > log_max)
	{
		log.pop();
	}

	return requestValue{ inversionData{ getData->GetType(), getData->GetColor(), getData->GetPos() } };
}



// データのリクエスト受理受信 // CheckRecvDataでackタイプだと分かってから呼ぶ
int NetworkUDP::RecvAck()
{
	ackPacket data;
	int result = NetWorkRecvUDP(GetUDPSocket(), NULL, NULL, &data, sizeof(data), FALSE);

	if (result < 0)
	{
		return result;
	}

	requestLists.erase(data.id);	// idに対応するコミット待ちだったデータを削除

	return 0;
}

// コミットされていないリクエストデータの再送
int NetworkUDP::ReSendUnCommitedRequestDatas()
{
	// 現在の時間
	auto nowtime = std::chrono::high_resolution_clock::now();

	for (auto& [key, value] : requestLists)
	{
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - value->time);

		if (duration <= timeout)
		{
			continue;
		}
		
		// 最初に送信された時間からタイムアウト分の時間が経過していたらデータを再送する
		auto data = value->data;
		int result = NetWorkSendUDP(GetUDPSocket(), GetPeerIP(), port, &data, sizeof(data));

		if (result < 0)
		{
			return result;
		}

		value->time = nowtime;	// タイムアウトで再送したデータの送信時刻を再設定する

		continue;
	}

	return 0;
}

// idの取得
int NetworkUDP::GetId()
{
	return netid.fetch_add(1);
}

// private /////////////////////////////////////////////////////

// idがログに存在するか
bool NetworkUDP::IsIdInLog(int id)
{
	std::queue<int> _log = log;	// 現状のログをコピー(いうて上限20なので)
	while (!_log.empty())
	{
		if (_log.front() == id)
		{
			return true;
		}
		_log.pop();
	}
	return false;
}