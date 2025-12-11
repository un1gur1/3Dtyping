#include <DxLib.h>
#include "Loading.h"

// コンストラクタ
Loading::Loading()
	: handle_(-1)
	, posX_(0)
	, posY_(0)
	, isLoading_(false)
	, loadTimer_(0)
{}

// デストラクタ
Loading::~Loading()
{}

// 初期化
void Loading::Init(void)
{
	loadTimer_ = 0;
	isLoading_ = false;
	posX_ = 0.0f;
	posY_ = 0.0f;
}

// 読み込み
void Loading::Load(void)
{
	handle_ = LoadGraph("Data/Image/Loading.png");
}

// 更新
void Loading::Update(void)
{
	loadTimer_++;

	// 読込中のものがなくなったら or 最低ロード時間経過
	if (GetASyncLoadNum() == 0 && loadTimer_ >= MIN_LOAD_TIME)
	{
		// ロード終了
		Init();
	}
	// 読み込み中
	else
	{
		// ロード画面を動作させるならここに記述
	}
}

// 描画
void Loading::Draw(void)
{
	DrawGraphF(
		posX_, posY_,	// 座標
		handle_,		// ハンドル
		true			// 透過フラグ
	);
}

// 解放
void Loading::Release(void)
{
	DeleteGraph(handle_);
}

// 非同期読み込みに切り替える
void Loading::StartAsyncLoad(void)
{
	isLoading_ = true;
	// 非同期読み込み開始
	SetUseASyncLoadFlag(true);
}

// 同期読み込みに切り替える
void Loading::EndAsyncLoad(void)
{
	// 非同期読み込み終了
	SetUseASyncLoadFlag(false);
}
