#pragma once
#include "../SceneBase.h"	
#include <string>
class AttackManager;
class TitleScene : public SceneBase
{
public:
	
	TitleScene(void);				// コンストラクタ
	~TitleScene(void) override;		// デストラクタ

	void Init(void)		override;	// 初期化
	void Load(void)		override;	// 読み込み
	void LoadEnd(void)	override;	// 読み込み後の処理
	void Update(void)	override;	// 更新
	void Draw(void)		override;	// 描画
	void Release(void)	override;	// 解放
	bool isRegisteringUltimate_ = false;
	int keyInputHandle_ = -1;
	char inputBuf_[128] = { 0 };
	std::string lastRegisteredCommand_;
	int scrollOffset_ = 0; // スクロール位置

	enum class PauseMenuState { None, Pause };
	PauseMenuState pauseState_ = PauseMenuState::None;
	int pauseCursor_ = 0;
	bool isPause_ = false;

private:
	AttackManager* attackManager_;
	int handle_;
};
