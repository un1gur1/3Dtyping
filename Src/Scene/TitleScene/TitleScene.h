#pragma once
#include "../SceneBase.h"	
#include <string>
#include <vector>
#include <unordered_map>

class AttackManager;

class TitleScene : public SceneBase
{
public:
	// --- コンストラクタ・デストラクタ ---
	TitleScene(void);
	~TitleScene(void) override;

	// --- SceneBase オーバーライド ---
	void Init(void)		override;
	void Load(void)		override;
	void LoadEnd(void)	override;
	void Update(void)	override;
	void Draw(void)		override;
	void Release(void)	override;

	// ポーズ状態（タイトルでも使う想定の場合）
	enum class PauseMenuState { None, Pause };

private:
	// --- CSV読み込み・コマンド処理 ---
	void LoadCommandsFromCSV(const std::string& path);
	void ProcessTitleCommand(const std::string& rawInput);

private:
	// --- システム・リソース ---
	AttackManager* attackManager_ = nullptr;
	int handle_ = -1; // 背景画像ハンドル

	// --- 状態フラグ ---
	bool isRegisteringUltimate_ = false;
	PauseMenuState pauseState_ = PauseMenuState::None;
	int pauseCursor_ = 0;
	bool isPause_ = false;

	// --- 入力・UI関連 ---
	int keyInputHandle_ = -1;         // 必殺技登録用の入力ハンドル
	char inputBuf_[128] = { 0 };      // 必殺技登録用のバッファ
	std::string inputHiraStr_;        // 登録用入力のひらがな結果

	int keyInputHandleCmd_ = -1;      // 常時表示のコマンド入力ハンドル
	char cmdInputBuf_[128] = { 0 };   // コマンド入力用のバッファ
	std::string cmdHiraStr_;          // コマンド入力のひらがな結果

	std::string lastRegisteredCommand_; // 最後に実行/登録したコマンドのステータス
	int scrollOffset_ = 0;            // 必殺技リストのスクロール位置

	// --- コマンド辞書 ---
	std::unordered_map<std::string, std::string> commandMap_;
	std::vector<std::string> commandNames_; // 表示用順序保持

	// --- メッセージ表示制御 ---
	int registeredDisplayRemaining_ = 0;
	static constexpr int REGISTERED_DISPLAY_FRAMES = 60; // 約1秒
	std::string registeredDisplayMessage_;

	// --- コマンド一覧表示モード ---
	bool showCommandList_ = false;
	std::vector<std::string> combinedCommandList_;
	int commandListScroll_ = 0;
	static constexpr int COMMAND_LIST_PAGE_SIZE = 20;

	// --- 入力制御用 ---
	bool ignoreNextReturn_ = false;
	bool prevReturnDown_ = false;
};