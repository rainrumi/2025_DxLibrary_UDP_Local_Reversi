//#
#pragma once
#include <DirectXMath.h>
#include <atomic>
#include <map>
#include <variant>
#include <chrono>
#include <queue>

#include "DxLib.h"
//https://dexall.co.jp/articles/?p=2063

using namespace DirectX;

struct requestData;
struct inversionData;
struct inversionPacket;
using requestValue = std::variant<inversionData, inversionPacket>;

enum class NetworkRole : int	// プレイヤーに与える役割
{
	NONE = 0,
	SERVER,
	CLIENT,
	MAX
};
enum class PackType : int
{
	NONE,//最近追加した
	REQUEST,
	ACK
};

class NetworkUDP
{
public:
	inline static constexpr int port = 1234;
	inline static std::chrono::milliseconds timeout{ 3000 };
private:
	static std::atomic<int> netid;
	int netUDPSocket;	// 自分のネットソケット
	IPDATA peerip;	// 相手のip
	IPDATA myip;		// 自分のip
	int role;			// 自分の役割
	std::map<int, std::unique_ptr<requestData>> requestLists;
	std::queue<int> log;	// 直近の受信したID
	inline static constexpr int log_max = 20;


public:
	NetworkUDP(IPDATA, IPDATA, int, int);	// コンストラクタ
	~NetworkUDP();
	requestValue CreateinversionData(int, XMINT2);	// inversionDataの作成
	int GetUDPSocket();	// ネットソケットの取得
	IPDATA GetPeerIP();	// 相手のIP取得
	IPDATA GetMyIP();	// 自身のIP取得
	int GetRole() const;	// 自身の役割取得
	int CheckRecvData();	// データの受信情報確認
	int SendData(requestValue&); // データの送信
	requestValue RecvRequest(); // データの受信 // データが存在しないとエラーになるのでCheckRecvDataで確認してから入るように
	int RecvAck();	// データのリクエスト受理受信
	int ReSendUnCommitedRequestDatas();	// コミットされていないリクエストデータの再送
	static int GetId();	// idの取得

private:
	bool IsIdInLog(int id);	// idがログに存在するか
};

/****************************************************************************/
struct inversionData	// 反転するときのデータ構造
{
public:
	const PackType type;
	const int id;
	const int color;
	const XMINT2 pos;
	inversionData(PackType _type, int _color, XMINT2 _pos)
		:
		type(_type),	// パッケージタイプ
		id(NetworkUDP::GetId()),	// idを増やして代入
		color(_color),
		pos(_pos)
	{ }
};
/****************************************************************************/
struct inversionPacket	// 反転するときのデータ構造パケット
{
public:
	PackType type;
	int id;
	int color;
	XMINT2 pos;
};
/****************************************************************************/
struct ackData	// データ受信したときのデータ構造
{
public:
	const PackType type;
	const int id;
	ackData(int _id)
		:
		type(PackType::ACK),
		id(_id)
	{ }
};
/****************************************************************************/
struct ackPacket	// データ受信したときのデータ構造パケット
{
public:
	PackType type;
	int id;
};
/****************************************************************************/
					// リクエストデータの汎用構造体
struct requestStruct
{
public:
	requestValue data;
	requestStruct(requestValue _data)
		:
		data(_data)
	{
	}

	int GetID() const
	{
		return std::visit([](auto const& v) {
			return v.id;  // 全構造体が id を持ってる前提
			}, data);
	}

	PackType GetType() const
	{
		return std::visit([](auto const& v) {
			return v.type;  // 全構造体が type を持ってる前提
			}, data);
	}

	int GetColor() const
	{
		return std::visit([](auto const& v) {
			return (int)v.color;  // 全構造体が color を持ってる前提
			}, data);
	}

	XMINT2 GetPos() const
	{
		return std::visit([](auto const& v) {
			return (XMINT2)v.pos;  // 全構造体が color を持ってる前提
			}, data);
	}
};
/****************************************************************************/
					// コミット前のデータを保持するデータ構造
struct requestData	
{
public:
	std::chrono::high_resolution_clock::time_point time;
	requestStruct data;
	requestData(std::chrono::high_resolution_clock::time_point _time, requestValue _data)
		:
		time(_time),
		data(_data)
	{ }
};
/****************************************************************************/
