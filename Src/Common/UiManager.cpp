#include "UiManager.h"
#include"../Application.h"
#include <DxLib.h>
#include "../Input/InputManager.h"
#include <algorithm>
// コンストラクタ

void UIManager::InitGrids()
{
    grids.clear();
    const int gridCountX = 5;
    const int gridCountY = 5;
    const float offset = 400.0f; // マス間隔

    // 【ここがポイント】
    // 中心を (0,0) にしたい場合、左上(開始地点)は 2マス分マイナス方向にずらす
    // 計算： 0.0f - (70.0f * 2) = -140.0f
    const float startX = -((gridCountX - 1) / 2.0f) * offset;
    const float startY = -((gridCountY - 1) / 2.0f) * offset;

    for (int y = 0; y < gridCountY; ++y)
    {
        for (int x = 0; x < gridCountX; ++x)
        {
            Grid grid;
            // これで x=2, y=2 (13個目) のときに posX=0, posY=0 になる
            float posX = startX + x * offset;
            float posY = startY + y * offset;

            grid.Init(posX, posY);
            grids.push_back(grid);
        }
    }
}
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

    // ポーズ中はUI以外の更新を止める
    if (state_ == UIState::Pause) {
        // 必要ならポーズ用のUI更新だけここに書く
        return;
    }

    if (!isGridInitialized) {
        InitGrids();
        isGridInitialized = true;
    }

    if (!grids.empty() && debugGridIndex_ < grids.size()) {
        grids[debugGridIndex_].pos_.x += debugOffsetX_;
        grids[debugGridIndex_].pos_.y += debugOffsetY_;
    }
    debugOffsetX_ = 0.0f;
    debugOffsetY_ = 0.0f;
}


// 毎フレーム描画
void UIManager::Draw(UIState currentState) {

   for (auto& grid : grids)
    {
        grid.Draw();
    }
  /*  if (!grids.empty()) {
        grids[0].Draw();
    }*/
    // 状態に応じて描画

   //    // デバッグ表示
   //if (!grids.empty() && debugGridIndex_ < grids.size()) {
   //    int x = static_cast<int>(grids[debugGridIndex_].pos_.x);
   //    int y = static_cast<int>(grids[debugGridIndex_].pos_.y);
   //    DrawFormatString(50, 50, 0xffffff, "Grid[%d] Pos: (%d, %d)", debugGridIndex_, x, y);
   //}
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
    //std::string playerHpText = "Player HP: " + std::to_string(playerHp_) + "/" + std::to_string(playerMaxHp_);
    //DrawString(INPUT_TEXT_X, INPUT_TEXT_Y, playerHpText.c_str(), COLOR_WHITE);

    //std::string enemyHpText = "Enemy HP: " + std::to_string(enemyHp_) + "/" + std::to_string(enemyMaxHp_);
    //DrawString(INPUT_TEXT_X, HIRA_TEXT_Y, enemyHpText.c_str(), COLOR_WHITE);

    //std::string enemyNameText = "Enemy: " + enemyName_;
    //DrawString(INPUT_TEXT_X, PREV_INPUT_Y, enemyNameText.c_str(), COLOR_WHITE);

        // プレイヤーHPバー
    DrawHpBar(50, 30, 300, 20, playerHp_, playerMaxHp_, GetColor(0, 128, 255), "PLAYER HP");

    // 敵HPバー
    DrawHpBar(50, 70, 300, 20, enemyHp_, enemyMaxHp_, GetColor(255, 64, 64), "ENEMY HP");

    //// 敵の名前
    //std::string enemyNameText = "Enemy: " + enemyName_;
    //DrawString(50, 100, enemyNameText.c_str(), COLOR_WHITE);
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
    // 半透明描画開始
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128); // 128は透明度（0:完全透明, 255:不透明）

    DrawBox(400, 100, 1600, 900, GetColor(0, 0, 0), TRUE); // 黒の半透明背景

    // 半透明描画終了（通常描画に戻す）
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawString(450, 220, "ポーズメニュー", GetColor(255, 255, 0));
    DrawString(410, 280, (pauseCursor_ == 0 ? "→ " : "   "), GetColor(255, 255, 255));
    DrawString(450, 280, "コマンド一覧", GetColor(255, 255, 255));
    DrawString(410, 320, (pauseCursor_ == 1 ? "→ " : "   "), GetColor(255, 255, 255));
    DrawString(450, 320, "タイトルに戻る", GetColor(255, 255, 255));
    DrawString(410, 360, (pauseCursor_ == 2 ? "→ " : "   "), GetColor(255, 255, 255));
    DrawString(450, 360, "ゲームを終了", GetColor(255, 255, 255));


    if (pauseCursor_ == 0) {
        int y = 180;
        int colW = 180; // 列の幅
        int maxPerCol = 13; // 1列に表示する最大数
        int xBase = 600;

        // --- 通常コマンド ---
        DrawString(xBase, y, "【通常コマンド】", GetColor(200, 255, 200));
        y += 30;
        for (size_t i = 0; i < normalCommandList_.size(); ++i) {
            int col = i / maxPerCol;
            int row = i % maxPerCol;
            int drawX = xBase + col * colW;
            int drawY = y + row * 24;
            DrawString(drawX, drawY, normalCommandList_[i].c_str(), GetColor(200, 200, 255));
        }
        y += maxPerCol * 24 + 20;

        // --- 必殺技コマンド ---
        DrawString(xBase, y, "【必殺技コマンド】", GetColor(255, 200, 200));
        y += 30;
        for (size_t i = 0; i < ultimateCommandList_.size(); ++i) {
            int col = i / maxPerCol;
            int row = i % maxPerCol;
            int drawX = xBase + col * colW;
            int drawY = y + row * 24;
            DrawString(drawX, drawY, ultimateCommandList_[i].c_str(), GetColor(255, 200, 200));
        }
    }
    //DrawFormatString(700, 500, 0xFF0000, "normal:%d ultimate:%d", normalCommandList_.size(), ultimateCommandList_.size());
}




void UIManager::SetGridState(int index, Grid::GridState state, bool isPlayerSide) {
    if (isPlayerSide) {
        if (index >= 0 && index < playerGrids_.size()) playerGrids_[index].state_ = state;
    }
    else {
        if (index >= 0 && index < enemyGrids_.size()) enemyGrids_[index].state_ = state;
    }
}
void UIManager::DrawHpBar(int x, int y, int width, int height, int hp, int maxHp, unsigned int color, const char* label)
{
    float rate = (float)hp / (float)maxHp;
    int barW = static_cast<int>(width * rate);
    DrawBox(x, y, x + width, y + height, GetColor(80, 80, 80), TRUE); // 背景
    DrawBox(x, y, x + barW, y + height, color, TRUE); // HP
    DrawFormatString(x, y - 20, 0xFFFFFF, "%s: %d / %d", label, hp, maxHp);
}
void UIManager::SetNormalCommandList(const std::vector<std::string>& list) {
    normalCommandList_ = list;
}
void UIManager::SetUltimateCommandList(const std::vector<std::string>& list) {
    ultimateCommandList_ = list;
}