#include "ResultScene.h"
#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"

ResultScene::ResultScene(void)
{
}

ResultScene::~ResultScene(void)
{
}

void ResultScene::Init(void)
{
	// 初期パラメータ
	alpha_ = 0.0f;
	fadingIn_ = true;
	fadingOut_ = false;
	promptTimer_ = 0.0f;
	pulseTimer_ = 0.0f;
	transitionRequested_ = false;
}

void ResultScene::Load(void)
{
	// 画像ロード（失敗時は handle_ が -1 になる）
	handle_ = LoadGraph("Data/Image/Lose.png");
}

void ResultScene::LoadEnd(void)
{
	Init();
}

void ResultScene::Update(void)
{
	const float dt = 1.0f / 60.0f; // 固定フレーム想定

	// フェードイン
	if (fadingIn_) {
		alpha_ += fadeSpeed_ * dt;
		if (alpha_ >= 255.0f) {
			alpha_ = 255.0f;
			fadingIn_ = false;
		}
	}

	// 点滅タイマー（プロンプト）
	promptTimer_ += dt;
	if (promptTimer_ >= promptBlinkInterval_) {
		promptTimer_ = 0.0f;
		promptVisible_ = !promptVisible_;
	}

	// テキスト脈動タイマー
	pulseTimer_ += dt;

	// スペースで次のシーンへ（ただし、フェードアウト中は無効）
	if (!transitionRequested_ && InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE)) {
		// フェードアウト開始
		transitionRequested_ = true;
		fadingOut_ = true;
	}

	// フェードアウト処理（遷移）
	if (fadingOut_) {
		alpha_ -= fadeSpeed_ * dt;
		if (alpha_ <= 0.0f) {
			alpha_ = 0.0f;
			// フェードアウト完了したらシーン遷移
			SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::TITLE);
			return;
		}
	}
}

void ResultScene::Draw(void)
{
	// 背景黒
	SetBackgroundColor(0, 0, 0);

	// 画像をフェードで描画
	if (handle_ != -1) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha_));
		// 画面中央にゆっくり上下移動させて見栄えを良くする
		static float moveTimer = 0.0f;
		moveTimer += 1.0f / 60.0f;
		int offsetY = static_cast<int>(sinf(moveTimer * 1.2f) * 8.0f);
		DrawGraph((640 - 640), offsetY, handle_, TRUE); // 画面座標に合わせて必要なら調整
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// タイトルテキスト（YOU LOSE...）: 脈動する色
	{
		const int baseX = 50;
		const int baseY = 200;

		float pulse = (sinf(pulseTimer_ * 2.0f) * 0.5f + 0.5f); // 0..1
		int r = static_cast<int>(255);
		int g = static_cast<int>(100 + 80 * pulse);
		int b = static_cast<int>(100 + 80 * pulse);

		// 半透明でフェード連動
		int alphaColor = static_cast<int>(alpha_);
		int color = GetColor(r, g, b);

		// 影
		DrawString(baseX + 3, baseY + 3, "YOU LOSE...", GetColor(0, 0, 0));
		// メイン
		DrawString(baseX, baseY, "YOU LOSE...", color);
	}

	// サブテキスト（説明）
	{
		const int x = 50;
		const int y = 900;
		if (promptVisible_) {
			DrawString(x, y, "SPACEでタイトルへ", GetColor(255, 200, 200));
		}
		else {
			// 消えるときは薄めの表示（見やすさ）
			DrawString(x, y, "SPACEでタイトルへ", GetColor(120, 120, 120));
		}
	}

	// 小さなヒント（任意）
	DrawString(50, 980, "Rキーでリプレイ（実装があれば）", GetColor(200, 200, 200));
}

void ResultScene::Release(void)
{
	if (handle_ != -1) {
		DeleteGraph(handle_);
		handle_ = -1;
	}
}