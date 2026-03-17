#include "Loading.h"
#include "../../Application.h" 
#include<DxLib.h>
#include <cmath>
Loading::Loading() : handle_(-1), isLoading_(false), loadTimer_(0), angle_(0.0f), dotCount_(0), currentTipIdx_(0), initialLoadNum_(0) {
    // Tipsの内容を登録
    tips_ = {
        "【Tips】みぎ、ひだり、うえ、した等のコマンドで移動や回避が可能だ！",
        "【Tips】『こうげき』以外にも『はっしゃ』など複数の攻撃コマンドがあるぞ。",
        "【Tips】必殺技は自分で登録したコマンドを入力することで発動できる！",
        "【Tips】敵の攻撃が着弾する前に移動して回避しよう。"
    };
}

Loading::~Loading() {}

void Loading::Init(void) {
    loadTimer_ = 0;
    isLoading_ = false;
    angle_ = 0.0f;
    dotCount_ = 0;
    initialLoadNum_ = 0;
    // 起動するたびに違うTipsを出す
    currentTipIdx_ = GetRand(static_cast<int>(tips_.size()) - 1);
}

void Loading::Load(void) {
    handle_ = LoadGraph("Data/Image/Loading.png"); // 背景またはロゴ
}

void Loading::Update(void) {
    if (!isLoading_) return;

    loadTimer_++;
    angle_ += 0.1f; // ぐるぐる回す用の角度

    // LOADING... のドット演出 (30フレームごとに更新)
    if (loadTimer_ % 30 == 0) {
        dotCount_ = (dotCount_ + 1) % 4;
    }

    int remaining = GetASyncLoadNum();
    if (initialLoadNum_ == 0 && remaining > 0) initialLoadNum_ = remaining;

    // ロード完了判定
    if (remaining == 0 && loadTimer_ >= MIN_LOAD_TIME) {
        Init();
    }
}

void Loading::Draw(void) {
    if (!isLoading_) return;

    // 1. 背景
    DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);

    // 2. メイン画像
    if (handle_ != -1) {
        DrawRotaGraph(960, 540, 1.0, 0.0, handle_, TRUE);
    }

    // 3. LOADING演出
    std::string loadStr = "LOADING";
    for (int i = 0; i < dotCount_; ++i) loadStr += ".";
    DrawFormatString(1600, 950, GetColor(255, 255, 255), loadStr.c_str());

    // ぐるぐる回るアイコン（円弧風に点を描画）
    int cx = 1550, cy = 965, r = 20;
    DrawCircle(cx, cy, r, GetColor(100, 100, 100), FALSE);

    // 円弧の代用（点を描画）
    float start = angle_;
    float end = angle_ + 1.5f;
    int numDots = 20;
    for (int i = 0; i < numDots; ++i) {
        float t = static_cast<float>(i) / (numDots - 1);
        float theta = start + (end - start) * t;
        int x = static_cast<int>(cx + r * std::cos(theta));
        int y = static_cast<int>(cy + r * std::sin(theta));
        DrawCircle(x, y, 3, GetColor(255, 255, 255), TRUE);
    }

    // 4. Tipsエリア
    DrawBox(0, 850, 1920, 1080, GetColor(20, 20, 20), TRUE);
    DrawLine(0, 850, 1920, 850, GetColor(100, 150, 255));
    DrawFormatString(100, 920, GetColor(255, 255, 100), "GOAL:");
    DrawFormatString(100, 950, GetColor(255, 255, 255), tips_[currentTipIdx_].c_str());

    // 5. プログレスバー
    int barW = 1720;
    DrawBox(100, 1020, 100 + barW, 1030, GetColor(50, 50, 50), TRUE);
    if (initialLoadNum_ > 0) {
        float progress = 1.0f - (static_cast<float>(GetASyncLoadNum()) / initialLoadNum_);
        float timeProgress = (static_cast<float>(loadTimer_) < MIN_LOAD_TIME ? static_cast<float>(loadTimer_) / MIN_LOAD_TIME : 1.0f);
        float finalProgress = (progress < timeProgress ? progress : timeProgress);

        if (finalProgress < 0.0f) finalProgress = 0.0f;
        if (finalProgress > 1.0f) finalProgress = 1.0f;

        DrawBox(100, 1020, 100 + static_cast<int>(barW * finalProgress), 1030, GetColor(100, 200, 255), TRUE);
    }
}

void Loading::StartAsyncLoad(void) {
    isLoading_ = true;
    initialLoadNum_ = 0; // リセット
    SetUseASyncLoadFlag(TRUE);
}

void Loading::EndAsyncLoad(void) {
    SetUseASyncLoadFlag(FALSE);
}

void Loading::Release(void) {
    if (handle_ != -1) {
        DeleteGraph(handle_);
        handle_ = -1;
    }
}