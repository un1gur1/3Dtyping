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
}
void ResultScene::Load(void)
{
	handle_ = LoadGraph("Data/Image/Lose.png");

}
void ResultScene::LoadEnd(void)
{
	Init();
}
void ResultScene::Update(void)
{
	// スペースで次のシーンへ
	if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance()->ChangeScene(
			SceneManager::SCENE_ID::TITLE);
	}
}
void ResultScene::Draw(void)
{
	SetBackgroundColor(0, 0, 0);
	DrawGraph(0, 0, handle_, true);

	DrawString(50, 900, "YOU LOSE...\nSPACEでタイトルへ", GetColor(255, 100, 100));
}
void ResultScene::Release(void)
{
	DeleteGraph(handle_);
}

