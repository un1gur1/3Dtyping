#pragma once
#include "../SceneBase.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "../../Common/RomanjiConverter.h"

class AttackManager;
class TitleScene : public SceneBase
{
public:
	TitleScene(void);				// コンストラクタ
	~TitleScene(void) override;		// デストラクタ

	void Init(void)		override;	// 初期化
	void Load(void)		override;	// 読込
	void LoadEnd(void)	override;	// 読込完了
	void Update(void)	override;	// 更新
	void Draw(void)		override;	// 描画
	void Release(void)	override;	// 解放

	// 常時使う入力ハンドル／バッファ（コマンド入力バー）
	int keyInputHandle_ = -1;
	char inputBuf_[128] = { 0 };
	std::string lastRegisteredCommand_;
	int scrollOffset_ = 0; // 一覧スクロール

	enum class PauseMenuState { None, Pause };
	PauseMenuState pauseState_ = PauseMenuState::None;
	int pauseCursor_ = 0;
	bool isPause_ = false;

private:
	AttackManager* attackManager_;
	int handle_;

	RomanjiConverter romanjiConverter_;     // ローマ字→ひらがな変換器

	// 床コマンド辞書（CSV 読み込み先）
	std::unordered_map<std::string, std::string> floorCommandMap_;
	std::vector<std::string> floorCommands_;

	// コマンド読み込み・処理
	void LoadFloorCommands(const std::string& path);
	void ProcessFloorCommand(const std::string& rawInput);
};