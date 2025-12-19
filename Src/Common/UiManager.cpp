#include "UiManager.h"
#include"../Application.h"
#include <DxLib.h>
// コンストラクタ


// プレイヤー情報セット
void UIManager::SetPlayerStatus(int hp, int maxHp) {
    playerHp_ = hp;
    playerMaxHp_ = maxHp;
}

// 敵情報セット
void UIManager::SetEnemyStatus(int hp, int maxHp, const std::string& name) {
    enemyHp_ = hp;
    enemyMaxHp_ = maxHp;
    enemyName_ = name;
}

// タイピング情報セット
void UIManager::SetTypingData(const std::string& target, const std::string& input) {
    targetWord_ = target;
    inputWord_ = input;
}

// 相殺イベント情報セット
void UIManager::SetClashData(int currentPower, float remainingTime) {
    clashPower_ = currentPower;
    timer_ = remainingTime;
}

// ポーズカーソル位置セット
void UIManager::SetPauseCursor(int cursor) {
    pauseCursor_ = cursor;
}

// 毎フレーム呼び出し
void UIManager::Update(UIState currentState) {
    state_ = currentState;
    // 必要に応じてアニメーションやタイマー処理をここで
}

// 毎フレーム描画
void UIManager::Draw(UIState currentState) {
    // 状態に応じて描画
    DrawCommonHUD();

    switch (currentState) {
    case UIState::Normal:
    case UIState::Stun:
        DrawTypingArea();
        break;
    case UIState::Decision:
        // 必要なら選択肢UI描画
        break;
    case UIState::Clash:
        DrawClashEvent();
        break;
    case UIState::Pause:
        DrawPauseMenu();
        break;
    }
}

// HPバーや共通HUD
void UIManager::DrawCommonHUD() {
    std::string playerHpText = "Player HP: " + std::to_string(playerHp_) + "/" + std::to_string(playerMaxHp_);
    DrawString(INPUT_TEXT_X, INPUT_TEXT_Y, playerHpText.c_str(), COLOR_WHITE);

    std::string enemyHpText = "Enemy HP: " + std::to_string(enemyHp_) + "/" + std::to_string(enemyMaxHp_);
    DrawString(INPUT_TEXT_X, HIRA_TEXT_Y, enemyHpText.c_str(), COLOR_WHITE);

    std::string enemyNameText = "Enemy: " + enemyName_;
    DrawString(INPUT_TEXT_X, PREV_INPUT_Y, enemyNameText.c_str(), COLOR_WHITE);
}
// UiManager.cpp

void UIManager::SetTypingStrings(
    const std::string& input,
    const std::string& converted,
    const std::string& prevInput,
    const std::string& prevConverted
) {
    typingInput_ = input;
    typingConverted_ = converted;
    typingPrevInput_ = prevInput;
    typingPrevConverted_ = prevConverted;
}

void UIManager::DrawTypingArea() {
    int oldFontSize = GetFontSize();
    SetFontSize(FONT_SIZE_LARGE);

    // 入力中表示
    DrawFormatString(INPUT_TEXT_X, INPUT_TEXT_Y, COLOR_MAGENTA,
        "コマンド入力(ローマ字): %s", typingInput_.c_str());
    DrawFormatString(INPUT_TEXT_X, HIRA_TEXT_Y, COLOR_GREEN,
        "変換(ひらがな): %s", typingConverted_.c_str());

    // 直前の入力表示
    DrawFormatString(INPUT_TEXT_X, PREV_INPUT_Y, COLOR_GREEN,
        "直前の入力: %s（%s）", typingPrevInput_.c_str(), typingPrevConverted_.c_str());

    SetFontSize(oldFontSize);
}


// 相殺イベント
void UIManager::DrawClashEvent() {
    // 例: パワーと残り時間
    // DrawText("Clash Power: " + std::to_string(clashPower_), ...);
    // DrawText("Time: " + std::to_string(timer_), ...);
}

// ポーズメニュー
void UIManager::DrawPauseMenu() {
    // 例: ポーズメニューとカーソル
    // DrawText("Pause", ...);
    // DrawText("-> Continue", ...);
    // DrawText("   Exit", ...);
    // DrawText("Cursor: " + std::to_string(pauseCursor_), ...);
}
void UIManager::SetGridState(int index, Grid::GridState state, bool isPlayerSide)
{
    if (index < 0 || index >= 5) return; // 範囲外は無視

    if (isPlayerSide) {
        playerGridStates_[index] = state;
    }
    else {
        enemyGridStates_[index] = state;
    }
}
