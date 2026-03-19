#include "Application.h"

#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Common/FpsControl.h"
#include "Input/InputManager.h"
#include "Scene/SceneManager.h"
#include "../Src/Common/EffectManager.h" 

Application* Application::instance_ = nullptr;

const std::string Application::PATH_MODEL = "Data/Model/";

Application::Application(void)
{
	isInitFail_ = false;
	isReleaseFail_ = false;
	fps_ = nullptr;
}

Application::~Application(void)
{
}

void Application::Init(void)
{
	// アプリケーションの初期設定
	SetWindowText("2416077_橋本晴翔");

	// ウィンドウ関連
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);
	ChangeWindowMode(true);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseerの初期化
	if (Effekseer_Init(8000) == -1) {
		isInitFail_ = true;
		return;
	}

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	// 描画先画面を裏にする
	SetDrawScreen(DX_SCREEN_BACK);

	// キー制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();
	InputManager::GetInstance()->Init();


	// シーン管理初期化
	SceneManager::CreateInstance();
	SceneManager::GetInstance()->Init();

	// FPS初期化
	fps_ = new FpsControl;
	fps_->Init();
}

// ゲームループ
void Application::Run(void)
{
	// ゲームループ
	while (ProcessMessage() == 0)
	{
		// エスケープキーが押されたらゲーム終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) return;

		// フレームレート更新
		// 1/60秒経過していないなら再ループさせる
		if (!fps_->UpdateFrameRate()) continue;

		// 画面を初期化
		ClearDrawScreen();

		InputManager::GetInstance()->Update();	// 入力制御更新
		SceneManager::GetInstance()->Update();	// シーン管理更新
		SceneManager::GetInstance()->Draw();	// シーン管理描画

		fps_->CalcFrameRate();	// フレームレート計算
		//fps_->DrawFrameRate();	// フレームレート描画

		ScreenFlip();
	}
}

void Application::Delete(void)
{
	// ==========================================================
	// ★ 修正箇所：終了処理の「順番」を完璧に並べ替えました！
	// ==========================================================

	// 1. 入力制御を削除
	InputManager::GetInstance()->DeleteInstance();

	// 2. シーン管理解放・削除（ここで各シーンの Release() が安全に呼ばれる）
	SceneManager::GetInstance()->Delete();
	SceneManager::GetInstance()->DeleteInstance();

	// 3. シーンが全て消えた後で、マネージャーに残ったエフェクトを完全消去！
	EffectManager::GetInstance().Clear();

	// 4. ここでようやく Effekseer 本体を終了（スペルミス "Effkseer_End" も修正）
	//Effekseer_End();
	Effkseer_End();

	// フレームレート解放
	delete fps_;

	// 5. 一番最後に DxLib 終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}
}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

// 画面を揺らす
void Application::ShakeScreen(int power, int duration, bool isShakeX, bool isShakeY)
{
	if (power <= 0 || duration <= 0)
	{
		return; // 無効なパラメータ
	}

	shakePower_ = power;			// 画面揺らしのパワー
	shakeDuration_ = duration;		// 画面揺らしの持続時間
	isShakeX_ = isShakeX;			// X方向の揺らしを有効にするか
	isShakeY_ = isShakeY;			// Y方向の揺らしを有効にするか
}

// 画面揺らしを停止
void Application::StopShakeScreen(void)
{
	shakePower_ = 0;			// 画面揺らしのパワーをリセット
	shakeDuration_ = 0;			// 画面揺らしの持続時間をリセット
	shakeOffsetX_ = 0;			// Xオフセットをリセット
	shakeOffsetY_ = 0;			// Yオフセットをリセット
}