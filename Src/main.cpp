#include <DxLib.h>

#include "Application.h"

// WinMain関数
//---------------------------------
int WINAPI WinMain(
	_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{

	// インスタンスの生成
	Application::CreateInstance();
	Application::GetInstance()->Init();

	if (Application::GetInstance()->IsInitFail())
	{
		// 初期化失敗
		return -1;
	}

	// 実行
	Application::GetInstance()->Run();

	// 解放
	Application::GetInstance()->Delete();

	if (Application::GetInstance()->IsReleaseFail())
	{
		// 解放失敗
		return -1;
	}

	Application::GetInstance()->DeleteInstance();

	return 0;

}
