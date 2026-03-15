#include "UiManager.h"
#include"../Application.h"
#include <DxLib.h>
#include "../Input/InputManager.h"
#include <algorithm>
#include <cctype>
#include "RomanjiConverter.h"

// コンストラクタ

// 正規化ヘルパー（TitleScene と同等の挙動に合わせる）
namespace {
    static bool IsSpaceSafe(unsigned char ch) {
        return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
    }

    static std::string ToLowerTrim(const std::string& s) {
        std::string t = s;
        t.erase(t.begin(), std::find_if(t.begin(), t.end(), [](unsigned char ch) {
            return !IsSpaceSafe(ch);
            }));
        t.erase(std::find_if(t.rbegin(), t.rend(), [](unsigned char ch) {
            return !IsSpaceSafe(ch);
            }).base(), t.end());
        for (char& c : t) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc < 128) c = static_cast<char>(std::tolower(uc));
        }
        return t;
    }

    static bool IsLikelyRomanji(const std::string& s) {
        if (s.empty()) return false;
        bool hasAlpha = false;
        for (unsigned char c : s) {
            if (c >= 128) return false;
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                hasAlpha = true;
            }
        }
        return hasAlpha;
    }

    static std::string ConvertRomanjiToHiragana(const std::string& in) {
        RomanjiConverter conv;
        std::string filtered;
        for (unsigned char c : in) {
            if (c != ' ') {
                filtered += static_cast<char>(std::tolower(c));
            }
        }
        return conv.convert(filtered);
    }

    static std::string ConvertIfRomanji(const std::string& s) {
        if (IsLikelyRomanji(s)) {
            std::string hira = ConvertRomanjiToHiragana(s);
            if (!hira.empty()) return hira;
        }
        return s;
    }
}

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

// 新規: プレイヤースタミナ設定
void UIManager::SetPlayerStamina(int stamina, int maxStamina) {
    playerStamina_ = stamina;
    playerMaxStamina_ = maxStamina;
}

// 新規: 敵の詠唱文字列設定
void UIManager::SetEnemyCasting(const std::string& casting) {
    enemyCasting_ = casting;
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
     // }
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
    // --- 敵HPバー：画面最上部、細長く中央寄せ（目立つが邪魔にならない） ---
    int availableW = SCREEN_WIDTH - 200;
    if (availableW < 0) availableW = 0; // 念のため負の幅を防止
    int enemyBarW = (availableW < 1200) ? availableW : 1200;
    int enemyBarX = (SCREEN_WIDTH - enemyBarW) / 2;
    int enemyBarY = 8;
    DrawHpBar(enemyBarX, enemyBarY, enemyBarW, 12, enemyHp_, enemyMaxHp_, GetColor(255, 64, 64), "BOSS");

    // 敵の名前（バーの右側に小さめに表示）
    if (!enemyName_.empty()) {
        DrawFormatString(enemyBarX + enemyBarW + 10, enemyBarY - 2, 0xFFFFFF, "%s", enemyName_.c_str());
    }

    // --- プレイヤーHP・スタミナ：画面左下に配置（チラ見で確認できる） ---
    int leftX = 20;
    int hpY = SCREEN_HEIGHT - 80;
    DrawHpBar(leftX, hpY, 240, 18, playerHp_, playerMaxHp_, GetColor(0, 128, 255), "PLAYER HP");

    // スタミナバー（HPの下に小さく）
    int stamY = hpY + 24;
    float stamRate = (playerMaxStamina_ > 0) ? static_cast<float>(playerStamina_) / playerMaxStamina_ : 0.0f;
    int stamW = static_cast<int>(220 * stamRate);
    DrawBox(leftX, stamY, leftX + 220, stamY + 12, GetColor(40, 40, 40), TRUE);
    DrawBox(leftX, stamY, leftX + stamW, stamY + 12, GetColor(200, 200, 50), TRUE);
    DrawFormatString(leftX + 230, stamY - 2, 0xFFFFFF, "STAMINA: %d/%d", playerStamina_, playerMaxStamina_);
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

    // 背景ボックス（入力領域）
    int bx = INPUT_TEXT_X - 8;
    int by = INPUT_TEXT_Y - 8;
    int bw = 900;
    int bh = 140;
    DrawBox(bx, by, bx + bw, by + bh, GetColor(16, 16, 16), TRUE);
    DrawBox(bx + 2, by + 2, bx + bw - 2, by + bh - 2, GetColor(40, 40, 40), FALSE);

    // 入力中表示（ローマ字）
    DrawFormatString(INPUT_TEXT_X, INPUT_TEXT_Y, COLOR_MAGENTA,
        "コマンド入力(ローマ字): %s", typingInput_.c_str());

    // ひらがな変換表示（UI側でも正規化して表示）
    std::string displayConverted = typingConverted_;
    if (displayConverted.empty()) {
        // もし converted が空ならローカルで正規化して表示
        displayConverted = ConvertIfRomanji(ToLowerTrim(typingInput_));
    }
    DrawFormatString(INPUT_TEXT_X, HIRA_TEXT_Y, COLOR_GREEN,
        "変換(ひらがな): %s", displayConverted.c_str());

    // 直前の入力表示（ローマ字 + 正規化）
    std::string prevConverted = typingPrevConverted_;
    if (prevConverted.empty()) {
        prevConverted = ConvertIfRomanji(ToLowerTrim(typingPrevInput_));
    }
    DrawFormatString(INPUT_TEXT_X, PREV_INPUT_Y, COLOR_GREEN,
        "直前の入力: %s（%s）", typingPrevInput_.c_str(), prevConverted.c_str());

    // 目標ワードと現在入力の一致状況（左下に小さく）
    if (!targetWord_.empty()) {
        std::string tnorm = ConvertIfRomanji(ToLowerTrim(targetWord_));
        std::string inorm = ConvertIfRomanji(ToLowerTrim(inputWord_));
        DrawFormatString(INPUT_TEXT_X, HIRA_TEXT_Y + 32, 0xFFFFAA,
            "目標: %s  入力(正規化): %s", tnorm.c_str(), inorm.c_str());
    }

    // --- 敵の詠唱窓（入力窓のすぐ下） ---
    if (!enemyCasting_.empty()) {
        int castX = INPUT_TEXT_X;
        int castY = by + bh + 8; // 入力ボックスのすぐ下
        int castW = bw;
        int castH = 48;
        // 赤い背景で「危険」を直感的に表示
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
        DrawBox(castX, castY, castX + castW, castY + castH, GetColor(60, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        DrawFormatString(castX + 8, castY + 8, GetColor(255, 80, 80),
            "ENEMY CAST: %s", enemyCasting_.c_str());
    }

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
        int colW = 220; // 列の幅を調整
        int maxPerCol = 12; // 1列に表示する最大数
        int xBase = 600;

        // --- 通常コマンド ---
        DrawString(xBase, y, "【通常コマンド】", GetColor(200, 255, 200));
        y += 30;
        for (size_t i = 0; i < normalCommandList_.size(); ++i) {
            int col = i / maxPerCol;
            int row = i % maxPerCol;
            int drawX = xBase + col * colW;
            int drawY = y + row * 24;
            // 表示はオリジナルと正規化（ひらがな）を併記
            std::string orig = normalCommandList_[i];
            std::string norm = ConvertIfRomanji(ToLowerTrim(orig));
            DrawString(drawX, drawY, orig.c_str(), GetColor(200, 200, 255));
            DrawFormatString(drawX + 130, drawY, GetColor(150, 255, 180), "(%s)", norm.c_str());
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
            std::string orig = ultimateCommandList_[i];
            std::string norm = ConvertIfRomanji(ToLowerTrim(orig));
            DrawString(drawX, drawY, orig.c_str(), GetColor(255, 200, 200));
            DrawFormatString(drawX + 130, drawY, GetColor(255, 230, 160), "(%s)", norm.c_str());
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
    if (maxHp <= 0) rate = 0.0f;
    int barW = static_cast<int>(width * rate);
    DrawBox(x, y, x + width, y + height, GetColor(80, 80, 80), TRUE); // 背景
    DrawBox(x, y, x + barW, y + height, color, TRUE); // HP
    DrawFormatString(x, y - 14, 0xFFFFFF, "%s: %d / %d", label, hp, maxHp);
}
void UIManager::SetNormalCommandList(const std::vector<std::string>& list) {
    normalCommandList_ = list;
}
void UIManager::SetUltimateCommandList(const std::vector<std::string>& list) {
    ultimateCommandList_ = list;
}