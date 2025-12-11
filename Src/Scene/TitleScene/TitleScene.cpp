#include "TitleScene.h"

#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"

TitleScene::TitleScene(void)
{
	handle_ = -1;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
}

void TitleScene::Load(void)
{
	handle_ = LoadGraph("Data/Image/Title.png");
}

void TitleScene::LoadEnd(void)
{
	Init();
}

void TitleScene::Update(void)
{
	// スペースが押されると次のシーンへ
	if(InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE))
	{
		// ゲームシーンへ
		SceneManager::GetInstance()->ChangeScene(
			SceneManager::SCENE_ID::GAME);
	}
}

void TitleScene::Draw(void)
{
	DrawGraph(0, 0, handle_, true);
}

void TitleScene::Release(void)
{
	DeleteGraph(handle_);
}
