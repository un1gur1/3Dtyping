#pragma once

#include <string>
#include <vector>
#include "../Application.h"
#include "Grid.h"

class Camera;

class UIManager {
public:
	// --- UIのステート ---
	enum class UIState {
		Normal,     // 通常戦闘
		Decision,   // 選択
		Clash,      // 相殺・鍔迫り合い
		Stun,       // スタン（ひるみ）
		Pause       // ポーズ画面
	};

	// シングルトンインスタンス取得
	static UIManager& GetInstance() {
		static UIManager instance;
		return instance;
	}

	// コピー・代入禁止
	UIManager(const UIManager&) = delete;
	UIManager& operator=(const UIManager&) = delete;

	// --- 初期化・更新・描画 ---
	void InitGrids();
	void Update(UIState currentState);

	// 描画メソッドはメンバ変数を変更しないため const 修飾
	void Draw(UIState currentState) const;

	// --- データセット関数 ---
	void SetPlayerStatus(int hp, int maxHp);
	void SetPlayerStamina(int stamina, int maxStamina);
	void SetEnemyStatus(int hp, int maxHp, const std::string& name);

	void SetEnemyCasting(const std::string& casting, float elapsed = 0.0f, float wait = 0.0f);
	void SetEnemyWaitState(bool isWaiting, float cooldown);

	void SetTypingData(const std::string& target, const std::string& input);
	void SetTypingStrings(const std::string& input, const std::string& converted, const std::string& prevInput, const std::string& prevConverted);
	void SetClashData(int currentPower, float remainingTime);
	void SetPauseCursor(int cursor);

	void SetNormalCommandList(const std::vector<std::string>& list);
	void SetUltimateCommandList(const std::vector<std::string>& list);
	void SetGridState(int index, Grid::GridState state, bool isPlayerSide);

	// --- UI定数 ---
	static constexpr int SCREEN_WIDTH = Application::SCREEN_SIZE_X;
	static constexpr int SCREEN_HEIGHT = Application::SCREEN_SIZE_Y;
	static constexpr unsigned int COLOR_MAGENTA = 0xff00ff;
	static constexpr unsigned int COLOR_GREEN = 0x00ff00;
	static constexpr unsigned int COLOR_WHITE = 0xffffff;
	static constexpr unsigned int COLOR_BLACK = 0x000000;

	// --- グリッド・データ（外部参照用） ---
	std::vector<Grid> playerGrids_ = std::vector<Grid>(5);
	std::vector<Grid> enemyGrids_ = std::vector<Grid>(5);
	std::vector<Grid> grids_; // 命名規則適用: grids -> grids_
	std::vector<std::string> normalCommandList_;
	std::vector<std::string> ultimateCommandList_;

private:
	UIManager() = default;
	~UIManager() = default;

	// --- 内部描画関数（すべて const 修飾） ---
	void DrawCommonHUD() const;
	void DrawTypingArea() const;
	void DrawClashEvent() const;
	void DrawPauseMenu() const;
	void DrawHpBar(int x, int y, int width, int height, int hp, int maxHp, unsigned int color, const char* label) const;

private:
	// --- 状態フラグ ---
	bool isGridInitialized_ = false; // 命名規則適用
	UIState state_ = UIState::Normal;

	// --- プレイヤー情報 ---
	int playerHp_ = 0;
	int playerMaxHp_ = 0;
	int playerStamina_ = 0;
	int playerMaxStamina_ = 0;

	// --- 敵情報 ---
	int enemyHp_ = 0;
	int enemyMaxHp_ = 0;
	std::string enemyName_;
	std::string enemyCasting_;
	float enemyCastingElapsed_ = 0.0f;
	float enemyCastingWait_ = 0.0f;
	bool enemyIsWaiting_ = false;
	float enemyCooldown_ = 0.0f;

	// --- タイピング情報 ---
	std::string targetWord_;
	std::string inputWord_;
	std::string typingInput_;
	std::string typingConverted_;
	std::string typingPrevInput_;
	std::string typingPrevConverted_;

	// --- その他システム ---
	int clashPower_ = 0;
	float timer_ = 0.0f;
	int pauseCursor_ = 0;
	Camera* camera_ = nullptr; // 命名規則適用

	int debugGridIndex_ = 0;
	float debugOffsetX_ = 0.0f;
	float debugOffsetY_ = 0.0f;
};