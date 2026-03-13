#pragma once
#include "../SceneBase.h"	
#include <string>
#include <vector>
#include <unordered_map>

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

	// 常時表示する入力窓
	int keyInputHandleCmd_ = -1;    // 入力窓用ハンドル
	char cmdInputBuf_[128] = { 0 }; // 入力窓バッファ

	// 変換表示用バッファ（リアルタイム変換結果を保持）
	std::string cmdHiraStr_;    // メイン入力のローマ字→ひらがな結果
	std::string inputHiraStr_;  // 登録モード入力のローマ字→ひらがな結果

	// CSV から参照するコマンド辞書 (name -> type)
	std::unordered_map<std::string, std::string> commandMap_;
	std::vector<std::string> commandNames_; // 表示用順序保持

	// 登録表示用（登録成功時に数秒表示）
	int registeredDisplayRemaining_ = 0;
	static constexpr int kRegisteredDisplayFrames = 60; // 約1秒(60fps 想定)
	std::string registeredDisplayMessage_;

	// コマンド一覧表示モード
	bool showCommandList_ = false;
	std::vector<std::string> combinedCommandList_;
	int commandListScroll_ = 0;
	static constexpr int kCommandListPageSize = 20;

	// 一覧遷移時の Enter 判定制御（判定の食い違い防止）
	bool ignoreNextReturn_ = false;
	// 前フレームの Return 押下状態（エッジ判定用）
	bool prevReturnDown_ = false;

	// CSV 読み込み
	void LoadCommandsFromCSV(const std::string& path);

	// 入力文字列を処理
	void ProcessTitleCommand(const std::string& rawInput);
};