#include "ResultWinScene.h"
#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"

ResultWinScene::ResultWinScene(void)
{
}

ResultWinScene::~ResultWinScene(void)
{
}

void ResultWinScene::Init(void)
{
}

void ResultWinScene::Load(void)
{
}

void ResultWinScene::LoadEnd(void)
{
	handle_ = LoadGraph("Data/Image/win.png");
	Init();
}

void ResultWinScene::Update(void)
{
	// スペースで次のシーンへ
	if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance()->ChangeScene(
			SceneManager::SCENE_ID::TITLE);
	}
}

void ResultWinScene::Draw(void)
{
	SetBackgroundColor(0, 0, 0);
	DrawGraph(0, 0, handle_, true);

	DrawString(50, 900, "YOU WIN\nSPACEでタイトルへ", GetColor(255, 100, 100));
}

void ResultWinScene::Release(void)
{
}
