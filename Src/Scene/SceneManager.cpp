#include "SceneManager.h"
#include <DxLib.h>
#include "Loading/Loading.h"
#include "TitleScene/TitleScene.h"
#include "GameScene/GameScene.h"
#include "ResultScene/ResultScene.h"
#include "ResultScene/ResultWinScene.h"
#include "../Common/Fader.h" 

SceneManager* SceneManager::instance_ = nullptr;

SceneManager::SceneManager(void)
{
	scene_ = nullptr;
	load_ = nullptr;
	fader_ = nullptr;
	sceneId_ = SCENE_ID::NONE;
	nextSceneId_ = SCENE_ID::NONE;
	isGameEnd_ = false;
	isChanging_ = false;
}

SceneManager::~SceneManager(void)
{
}

void SceneManager::Init(void)
{
	// ロード画面生成
	load_ = new Loading();
	load_->Init();
	load_->Load();

	// フェード生成
	fader_ = new Fader();
	fader_->Init();

	// 3D情報の初期化
	Init3D();

	// 最初はフェードなしでタイトルへ
	isChanging_ = false;
	ChangeScene(SCENE_ID::TITLE);
}

void SceneManager::Init3D(void)
{
	SetBackgroundColor(0, 0, 0);
	SetUseZBuffer3D(true);
	SetWriteZBuffer3D(true);
	SetUseBackCulling(true);
	SetUseLighting(true);
	ChangeLightTypeDir({ 0.00f, -1.00f, 1.00f });
}

void SceneManager::Update(void)
{
	fader_->Update();

	// 1. シーン切り替え中（暗転待ち）の処理
	if (isChanging_)
	{
		// 画面が真っ暗になったら
		if (fader_->GetState() == Fader::STATE::FADE_OUT && fader_->IsEnd())
		{
			// --- ここでシーンを入れ替える ---
			PerformSceneChange();

			// ロードが「終わるのを待たずに」、すぐにフェードインを開始する
			fader_->SetFade(Fader::STATE::FADE_IN);

			isChanging_ = false; // 暗転待ちフラグを下ろす
		}
		return; // 暗転中はここで処理を抜ける
	}

	if (scene_ == nullptr) { return; }

	// 2. ロード中の処理（画面は明るくなりながら、裏でロードが進む）
	if (load_->IsLoading())
	{
		load_->Update();

		if (load_->IsLoading() == false)
		{
			scene_->LoadEnd();
			// ここではもうフェードイン指示はしない（上で既にやってるから）
		}
	}
	else
	{
		scene_->Update();
	}
}
void SceneManager::Draw(void)
{
	// 1. シーンまたはロード画面を描画
	if (load_->IsLoading())
	{
		load_->Draw();
	}
	else if (scene_ != nullptr)
	{
		scene_->Draw();
	}

	fader_->Draw();
}

// 外部（TitleSceneなど）から呼ばれる遷移リクエスト
void SceneManager::ChangeScene(SCENE_ID nextId)
{
	if (isChanging_) return; // 連続押し防止

	// 初回（起動時）はフェードなしで即切り替え
	if (sceneId_ == SCENE_ID::NONE)
	{
		nextSceneId_ = nextId;
		PerformSceneChange();
	}
	else
	{
		// 2回目以降は遷移を予約してフェードアウト開始
		nextSceneId_ = nextId;
		isChanging_ = true;
		fader_->SetFade(Fader::STATE::FADE_OUT);
	}
}
void SceneManager::Delete(void)
{
	if (scene_)
	{
		scene_->Release();
		delete scene_;
	}

	if (load_)
	{
		load_->Release();
		delete load_;
	}

	if (fader_)
	{
		// FaderにReleaseがあれば呼ぶ
		delete fader_;
	}
}


// 内部で実際にメモリを入れ替える処理
void SceneManager::PerformSceneChange(void)
{
	sceneId_ = nextSceneId_;

	// 現在のシーンを解放
	if (scene_ != nullptr)
	{
		scene_->Release();
		delete scene_;
		scene_ = nullptr;
	}

	// 各シーンに切り替え
	switch (sceneId_)
	{
	case SCENE_ID::TITLE:		scene_ = new TitleScene(); break;
	case SCENE_ID::GAME:		scene_ = new GameScene();  break;
	case SCENE_ID::RESULT_WIN:	scene_ = new ResultWinScene(); break;
	case SCENE_ID::RESULT_LOSE:	scene_ = new ResultScene(); break;
	default: break;
	}

	if (scene_)
	{
		// 読み込み(非同期)
		load_->StartAsyncLoad();
		scene_->Load();
		load_->EndAsyncLoad();
	}
}