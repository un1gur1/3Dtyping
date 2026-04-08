#pragma once
#include "../SceneBase.h"	
#include <string>

class ResultScene : public SceneBase
{
public:

	ResultScene(void);				// コンストラクタ
	~ResultScene(void) override;		// デストラクタ

	void Init(void)		override;	// 初期化
	void Load(void)		override;	// 読み込み
	void LoadEnd(void)	override;	// 読み込み完了
	void Update(void)	override;	// 更新
	void Draw(void)		override;	// 描画
	void Release(void)	override;	// 解放

private:
	int handle_ = -1;

	// フェード / 表示制御
	float alpha_ = 0.0f;             // 0..255
	bool fadingIn_ = true;
	bool fadingOut_ = false;
	const float fadeSpeed_ = 400.0f; // alpha change / second

	// プロンプト点滅
	float promptTimer_ = 0.0f;
	const float promptBlinkInterval_ = 0.6f;
	bool promptVisible_ = true;

	// テキスト脈動
	float pulseTimer_ = 0.0f;

	// 遷移制御
	bool transitionRequested_ = false;
};