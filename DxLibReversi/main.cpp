//#
#include "main.h"

#include <random>

#include "DxLib.h"
#include "Player.h"
#include "Reversi.h"
#include "NetworkUDP.h"
#include "ActionBehavior.h"
#include "Input.h"
#include "DataSet.h"

#include "Animation.h"

using namespace Resolution;

// ホストはクライアントが待機状態になってからデータを送る必要がある

// コンストラクタ
main::main()
	:
	reversi(std::make_unique <Reversi>()),
	input(std::make_unique <Input>()),
	localPlayer(nullptr),
	peerPlayer(nullptr),
	networkUdp(nullptr),
	font(CreateFontToHandle("Rounded Mplus 1c", 20, 3, DX_FONTTYPE_ANTIALIASING)),	// フォント初期化
	animation(nullptr)							// アニメーション初期化
{
	AddFontResourceExA("MPLUSRounded1c-Regular.ttf", FR_PRIVATE, NULL);	// フォントの作成
	return;
}

// デストラクタ
main::~main()
{
	DeleteFontToHandle(font[(int)FontType::RoundedMplus]);	// フォント削除
	RemoveFontResourceExA("", FR_PRIVATE, NULL);			// windowsに一時的に保持していたフォントデータを削除
};

// メインプロセス
void main::mainProc()
{
	// 自身の色
	int myColor = localPlayer->GetMyColor();

	// 自分の初期ステートを決める
	if (myColor == (int)pieceColor::Black)		Action::ChangeAction(action, ActionState::LocalTurn);
	else if (myColor == (int)pieceColor::White) Action::ChangeAction(action, ActionState::PeerTurn);
	else return;

	// マウスのインプット情報をリセットする
	input->ClearMouseClickedLog(input->putPiece);

	// ループ
	while (ClearDrawScreen() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0 && ProcessMessage() == 0)
	{
		//reversi->Update();	// リバーシアップデート
		action->Update(*reversi, *networkUdp, *input, myColor, font[(int)FontType::RoundedMplus]);	// アクションごとに違う処理の更新
		ActionState currentActionCached = action->GetRequestCurrentState();						// アクション内からアクションの変更リクエストを取得
		if (currentActionCached != ActionState::None && currentActionCached != ActionState::MAX)
		{
			Action::ChangeAction(action, currentActionCached);	// キャッシュに存在するアクションへ変更
			input->ClearMouseClickedLog(input->putPiece);	// マウスのインプット情報をリセットする
		}
		//networkUdp->ReSendUnCommitedRequestDatas();

		// 画面の描画
		reversi->ViewGrid();
		reversi->ViewState(myColor);
		reversi->ViewUI(myColor, font[(int)FontType::RoundedMplus]);

		ScreenFlip();	// フリップ
		WaitVSync(1);	// 垂直同期
	}
}

// IPの設定
IPDATA main::GetInputIp(const std::string& who)
{
	IPDATA ip(0, 0, 0, 0);

	int _ip[4] = { 0,0,0,0 };

	auto txt1Pos = RaruLib::ConvertNormalizedCenterToPixel(XMFLOAT2(0, 0));
	auto txt2Pos = RaruLib::ConvertNormalizedCenterToPixel(XMFLOAT2(0, -0.1));
	auto txtBoxPos = RaruLib::ConvertNormalizedCenterToPixel(XMFLOAT2(0, 0.1));

	// IPをセットする
	for (int i = 0; i < 4; i++)
	{
		bool valid = false;

		while (!valid && ClearDrawScreen() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0 && ProcessMessage() == 0)
		{
			ClearDrawScreen();

			char input[4];

			std::string msg ="「" + who + "」が使用するIPの第" + std::to_string(i + 1) + "オクテットを入力（失敗した場合再起動してください）";
			DrawStringToHandle(txt1Pos.x, txt1Pos.y, msg.c_str(), Color::black, font[(int)FontType::RoundedMplus]);
			std::string msg2 = "入力したIP：" + std::to_string(_ip[0]) + "," + std::to_string(_ip[1]) + "," + std::to_string(_ip[2]) + "," + std::to_string(_ip[3]);
			DrawStringToHandle(txt2Pos.x, txt2Pos.y, msg2.c_str(), Color::black, font[(int)FontType::RoundedMplus]);

			KeyInputString(txtBoxPos.x, txtBoxPos.y, 3, input, FALSE);

			bool allRange = true;
			// 全部数字か調べて、そうであれば抜ける
			for (int j = 0; input[j] != '\0'; j++)
			{
				if (input[j] < '0' || input[j] > '9')
				{
					allRange = false;
					break;
				}
			}

			if (allRange && input[0] != '\0')
			{
				int value = atoi(input);

				_ip[i] = value;
				valid = true;
			}

			ScreenFlip();	// フリップ
			WaitVSync(1);	// 垂直同期
		}
	}

	ip.d1 = _ip[0];
	ip.d2 = _ip[1];
	ip.d3 = _ip[2];
	ip.d4 = _ip[3];

	return ip;
}

// ネットワークの設定
void main::NetworkSet()
{
	bool isServer = false;

	/*
	AnimationData anim_data;
	bool load_success = LoadAnimationData("AnimationData.json", anim_data);
	if (!load_success || anim_data.key_frames.size() < 2)
	{
		// データの読み込みに失敗するか、キーフレームが足りない場合は終了
		MessageBox(NULL, "Failed to load animation data or not enough keyframes.", "Error", MB_OK);
		DxLib_End();
		return;
	}
	// アニメーション開始時刻
	long long start_time = GetNowCount();
	int current_color = GetColor(0, 0, 0); // 補間後の色格納用*/

	// ホスト（サーバー）かクライアントか決める
	while (ClearDrawScreen() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0 && ProcessMessage() == 0)
	{
		auto txt1Pos = RaruLib::ConvertNormalizedCenterToPixel(XMFLOAT2(0, 0));
		DrawStringToHandle(txt1Pos.x, txt1Pos.y,
			"ホストでプレイする場合は「H」\nクライアントでプレイする場合は「C」をプッシュ\nホストはクライアントが待機を始めてから起動する必要があります",
			Color::black, font[(int)FontType::RoundedMplus]);
		bool isPressedH = CheckHitKey(KEY_INPUT_H);
		bool isPressedC = CheckHitKey(KEY_INPUT_C);
		isServer = isPressedH;

		if (isPressedH || isPressedC)
		{
			break;
		}
		/*
		// 経過時間 (ミリ秒) と秒に変換
		long long elapsed_time_ms = GetNowCount() - start_time;
		double current_time = (double)elapsed_time_ms / 1000.0;

		// アニメーションが終了したら時間をリセット（ループ処理）
		if (current_time > anim_data.duration)
		{
			start_time = GetNowCount();
			current_time = 0.0;
		}

		// 補間を行う2つのキーフレームを特定
		KeyFrame frame1 = anim_data.key_frames[0]; // 最初のフレーム
		KeyFrame frame2 = anim_data.key_frames[1]; // 最後のフレーム

		// 現在の時間が2つのキーフレーム間にあるときの比率tを計算
		double t = current_time / (frame2.time - frame1.time);

		// 線形補間を実行
		double current_x = Lerp(frame1.pos_x, frame2.pos_x, t);
		double current_y = Lerp(frame1.pos_y, frame2.pos_y, t);
		double current_size = Lerp(frame1.size, frame2.size, t);

		// 色はR, G, Bそれぞれで補間してからDxLibカラーに変換
		int r = (int)Lerp(GetRValue(frame1.color), GetRValue(frame2.color), t);
		int g = (int)Lerp(GetGValue(frame1.color), GetGValue(frame2.color), t);
		int b = (int)Lerp(GetBValue(frame1.color), GetBValue(frame2.color), t);
		current_color = GetColor(r, g, b);

		// 補間結果を使って描画
		DrawBox(
			(int)(current_x - current_size / 2), // 左上のX座標
			(int)(current_y - current_size / 2), // 左上のY座標
			(int)(current_x + current_size / 2), // 右下のX座標
			(int)(current_y + current_size / 2), // 右下のY座標
			current_color,
			TRUE // 塗りつぶし
		);
		*/

		ScreenFlip();	// フリップ
		WaitVSync(1);	// 垂直同期
	} 

	localPlayer = std::make_unique <Player>("kakasi");	// 何かしらの方法で名前を決める

	int netUDPSocket = MakeUDPSocket(NetworkUDP::port);

	IPDATA myIp = GetInputIp("自分");
	IPDATA peerIp = GetInputIp("相手");

	// 現在はここに自分と相手のIPを入れる必要がある！！！！！！！！！！！！！！！！！
	networkUdp = std::make_unique<NetworkUDP>(myIp, peerIp,
		netUDPSocket, (isServer) ? (int)NetworkRole::SERVER : (int)NetworkRole::CLIENT);	// ネットの情報を作成//	自身がホストかサーバーか決める

	Action::ChangeAction(action, ActionState::ErrorState);	// ステート初期化

	if (isServer)
	{
		int min = 0;
		int max = 100;
		// ここの乱数初期化はどこかの構造体にするべき
		std::random_device seed_gen;
		std::mt19937 engine(seed_gen());
		std::uniform_int_distribution<> dist(min, max);
		int random = dist(engine) % 2;
		int myColor = (random == 0) ? (int)pieceColor::Black : (int)pieceColor::White;
		int peerColor = (random == 0) ? (int)pieceColor::White : (int)pieceColor::Black;
		localPlayer->SetMyColor(myColor);	// 自分の色を決める
		int result = NetWorkSendUDP(networkUdp->GetUDPSocket(), networkUdp->GetPeerIP(), NetworkUDP::port, &myColor, sizeof(myColor));
		peerPlayer = std::make_unique <Player>("Uremiss");	// 相手の情報を設定？
		peerPlayer->SetMyColor(peerColor);	// 相手の色を決める？

		int my = networkUdp->GetUDPSocket();
		IPDATA you = networkUdp->GetPeerIP();
	}
	else
	{
		// 相手が情報を送ってきたら受け取る
		while (ClearDrawScreen() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0 && ProcessMessage() == 0)
		{
			auto txt1Pos = RaruLib::ConvertNormalizedCenterToPixel(XMFLOAT2(0, 0));
			DrawStringToHandle(txt1Pos.x, txt1Pos.y, "接続中...\n（接続が成功しない場合、サーバー側が先に接続を試みた可能性があります。\nクライアントPCでのIP入力を先に行ってください）", Color::black, font[(int)FontType::RoundedMplus]);
			if (CheckNetWorkRecvUDP(networkUdp->GetUDPSocket()))
			{
				break;
			}
			ScreenFlip();	// フリップ
			WaitVSync(1);	// 垂直同期
		}
		int peerColor = 0;
		NetWorkRecvUDP(networkUdp->GetUDPSocket(), NULL, NULL, &peerColor, sizeof(peerColor), FALSE);
		int myColor = (peerColor == (int)pieceColor::Black) ? (int)pieceColor::White : (int)pieceColor::Black;
		localPlayer->SetMyColor(myColor);	// 自分の色を決める
		peerPlayer = std::make_unique <Player>("Uremiss_server");	// 相手の情報を設定
		peerPlayer->SetMyColor(peerColor);	// 相手の色を決める
	}

	return;
}

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (DxLib_Init() == -1)	// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	SetWindowSizeChangeEnableFlag(TRUE);		// ウィンドウサイズ可変
	SetAlwaysRunFlag(TRUE);						// 非アクティブ時も処理を続行
	ChangeWindowMode(TRUE);						// ウィンドウモード
	SetGraphMode(WND_WIDTH, WND_HEIGHT, 32);	// 解像度指定
	SetMouseDispFlag(TRUE);						// マウスカーソル表示
	SetBackgroundColor(255, 220, 233);			// 背景色変更
	SetDrawScreen(DX_SCREEN_BACK);				// 描画する場所を裏画面にする
												// テキストボックスの色を指定する
	SetKeyInputStringColor(
		GetColor(0, 0, 0),     // NmlStr : 通常入力文字
		GetColor(0, 0, 0),     // NmlCur : 通常カーソル
		GetColor(255, 255, 255), // IMEStrBack
		GetColor(0, 0, 0),     // IMECur
		GetColor(0, 0, 0),     // IMELine
		GetColor(0, 0, 0),     // IMESelectStr
		GetColor(0, 0, 0),     // IMEModeStr
		GetColor(0, 0, 0),     // NmlStrE
		GetColor(0, 0, 0),     // IMESelectStrE
		GetColor(0, 0, 0),     // IMEModeStrE
		GetColor(0, 0, 0),     // IMESelectWinE
		GetColor(255, 255, 255), // IMESelectWinF
		GetColor(180, 180, 180), // SelectStrBackColor
		GetColor(0, 0, 0),     // SelectStrColor
		GetColor(0, 0, 0),     // SelectStrEdgeColor
		GetColor(0, 0, 0),     // IMEStr
		GetColor(0, 0, 0)      // IMEStrE
	);


	main main;			// メインデータを持つクラス
	main.NetworkSet();	// ネットのセットアップ実行
	main.mainProc();	// メインプロセス実行

	DxLib_End();		// ＤＸライブラリ使用の終了処理

	return 0;			// ソフトの終了 
}