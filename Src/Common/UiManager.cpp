#include "UiManager.h"
#include "../Application.h"
#include <DxLib.h>
#include "../Input/InputManager.h"
#include <algorithm>
#include <cctype>
#include "RomanjiConverter.h"

// =======================================================
// 正規化・文字列ヘルパー（無名名前空間）
// =======================================================
namespace {
	static bool IsSpaceSafe(const unsigned char ch) {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}

	static std::string ToLowerTrim(const std::string& s) {
		std::string t = s;
		t.erase(t.begin(), std::find_if(t.begin(), t.end(), [](unsigned char ch) { return !IsSpaceSafe(ch); }));
		t.erase(std::find_if(t.rbegin(), t.rend(), [](unsigned char ch) { return !IsSpaceSafe(ch); }).base(), t.end());
		for (char& c : t) {
			const unsigned char uc = static_cast<unsigned char>(c);
			if (uc < 128) c = static_cast<char>(std::tolower(uc));
		}
		return t;
	}

	static bool IsLikelyRomanji(const std::string& s) {
		if (s.empty()) return false;
		bool hasAlpha = false;
		for (unsigned char c : s) {
			if (c >= 128) return false;
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) hasAlpha = true;
		}
		return hasAlpha;
	}

	static std::string ConvertRomanjiToHiragana(const std::string& in) {
		RomanjiConverter conv;
		std::string filtered;
		for (unsigned char c : in) {
			if (c != ' ') filtered += static_cast<char>(std::tolower(c));
		}
		return conv.convert(filtered);
	}

	static std::string ConvertIfRomanji(const std::string& s) {
		if (IsLikelyRomanji(s)) {
			const std::string hira = ConvertRomanjiToHiragana(s);
			if (!hira.empty()) return hira;
		}
		return s;
	}

	static size_t GetUtf8CharCount(const std::string& s) {
		size_t count = 0;
		for (unsigned char c : s) {
			if ((c & 0xC0) != 0x80) count++;
		}
		return count;
	}

	static std::string Utf8Substring(const std::string& s, const size_t n) {
		if (n == 0 || s.empty()) return std::string();
		std::string out;
		out.reserve(s.size());
		size_t chars = 0;
		for (size_t i = 0; i < s.size(); ) {
			const unsigned char c = static_cast<unsigned char>(s[i]);
			size_t len = 1;
			if (c < 0x80) len = 1;
			else if ((c & 0xE0) == 0xC0) len = 2;
			else if ((c & 0xF0) == 0xE0) len = 3;
			else if ((c & 0xF8) == 0xF0) len = 4;

			if (i + len > s.size()) len = s.size() - i;
			if (chars >= n) break;
			out.append(s.data() + i, len);
			i += len;
			chars++;
		}
		return out;
	}
}

// =======================================================
// データセット系
// =======================================================
void UIManager::SetPlayerStatus(const int hp, const int maxHp) { playerHp_ = hp; playerMaxHp_ = maxHp; }
void UIManager::SetPlayerStamina(const int stamina, const int maxStamina) { playerStamina_ = stamina; playerMaxStamina_ = maxStamina; }
void UIManager::SetEnemyStatus(const int hp, const int maxHp, const std::string& name) { enemyHp_ = hp; enemyMaxHp_ = maxHp; enemyName_ = name; }
void UIManager::SetTypingData(const std::string& target, const std::string& input) { targetWord_ = target; inputWord_ = input; }
void UIManager::SetClashData(const int currentPower, const float remainingTime) { clashPower_ = currentPower; timer_ = remainingTime; }
void UIManager::SetPauseCursor(const int cursor) { pauseCursor_ = cursor; }

void UIManager::SetEnemyCasting(const std::string& casting, const float elapsed, const float wait) {
	enemyCasting_ = casting;
	enemyCastingElapsed_ = elapsed;
	enemyCastingWait_ = wait;
}

void UIManager::SetEnemyWaitState(const bool isWaiting, const float cooldown) {
	enemyIsWaiting_ = isWaiting;
	enemyCooldown_ = cooldown;
}

void UIManager::SetTypingStrings(const std::string& input, const std::string& converted, const std::string& prevInput, const std::string& prevConverted) {
	typingInput_ = input;
	typingConverted_ = converted;
	typingPrevInput_ = prevInput;
	typingPrevConverted_ = prevConverted;
}

void UIManager::SetNormalCommandList(const std::vector<std::string>& list) { normalCommandList_ = list; }
void UIManager::SetUltimateCommandList(const std::vector<std::string>& list) { ultimateCommandList_ = list; }

void UIManager::SetGridState(const int index, const Grid::GridState state, const bool isPlayerSide) {
	if (isPlayerSide) {
		if (index >= 0 && index < playerGrids_.size()) playerGrids_[index].state_ = state;
	}
	else {
		if (index >= 0 && index < enemyGrids_.size()) enemyGrids_[index].state_ = state;
	}
}

// =======================================================
// システム更新・描画
// =======================================================
void UIManager::InitGrids() {
	grids_.clear();
	const int gridCountX = 5;
	const int gridCountY = 5;
	const float offset = 400.0f;

	const float startX = -((gridCountX - 1) / 2.0f) * offset;
	const float startY = -((gridCountY - 1) / 2.0f) * offset;

	for (int y = 0; y < gridCountY; ++y) {
		for (int x = 0; x < gridCountX; ++x) {
			Grid grid;
			const float posX = startX + x * offset;
			const float posY = startY + y * offset;
			grid.Init(posX, posY);
			grids_.push_back(grid);
		}
	}
}

void UIManager::Update(const UIState currentState) {
	state_ = currentState;
	if (state_ == UIState::Pause) return;

	if (!isGridInitialized_) {
		InitGrids();
		isGridInitialized_ = true;
	}

	if (!grids_.empty() && debugGridIndex_ < grids_.size()) {
		grids_[debugGridIndex_].pos_.x += debugOffsetX_;
		grids_[debugGridIndex_].pos_.y += debugOffsetY_;
	}
	debugOffsetX_ = 0.0f;
	debugOffsetY_ = 0.0f;
}

void UIManager::Draw(const UIState currentState) const {
	// 注意: Grid::Draw() が constメンバ関数でないとコンパイルエラーになる場合があります。
	// エラーが出る場合は Grid クラスの Draw() メソッドに const を付けてください。
	for (const auto& grid : grids_) {
		// grid.Draw(); // <- ここでエラーが出る場合は Grid 側の修正が必要です
		// ※一旦 const キャスト等で回避するか、Grid側を const 化してください
		const_cast<Grid&>(grid).Draw();
	}

	DrawCommonHUD();

	switch (currentState) {
	case UIState::Normal:
	case UIState::Stun:
		DrawTypingArea();
		break;
	case UIState::Decision:
		break;
	case UIState::Clash:
		DrawClashEvent();
		break;
	case UIState::Pause:
		DrawPauseMenu();
		break;
	}
}

// =======================================================
// 各種描画パーツ（すべて const 修飾済）
// =======================================================
void UIManager::DrawHpBar(const int x, const int y, const int width, const int height, const int hp, const int maxHp, const unsigned int color, const char* label) const {
	const float rate = (maxHp > 0) ? static_cast<float>(hp) / static_cast<float>(maxHp) : 0.0f;
	const int barW = static_cast<int>(width * rate);

	DrawBox(x, y, x + width, y + height, GetColor(80, 80, 80), TRUE);
	DrawBox(x, y, x + barW, y + height, color, TRUE);
	DrawFormatString(x, y - 16, COLOR_WHITE, "%s: %d / %d", label, hp, maxHp);
}

void UIManager::DrawCommonHUD() const {
	const int enemyBarW = (SCREEN_WIDTH > 1400) ? 1200 : SCREEN_WIDTH - 200;
	const int enemyBarX = (SCREEN_WIDTH - enemyBarW) / 2;
	const int enemyBarY = 24;
	DrawHpBar(enemyBarX, enemyBarY, enemyBarW, 16, enemyHp_, enemyMaxHp_, GetColor(255, 64, 64), "BOSS");

	if (!enemyName_.empty()) {
		DrawFormatString(enemyBarX + enemyBarW + 10, enemyBarY - 2, COLOR_WHITE, "%s", enemyName_.c_str());
	}

	const int leftX = 40;
	const int hpY = SCREEN_HEIGHT - 100;
	DrawHpBar(leftX, hpY, 300, 20, playerHp_, playerMaxHp_, GetColor(0, 128, 255), "PLAYER HP");

	const int stamY = hpY + 30;
	const float stamRate = (playerMaxStamina_ > 0) ? static_cast<float>(playerStamina_) / static_cast<float>(playerMaxStamina_) : 0.0f;
	const int stamW = static_cast<int>(260 * stamRate);
	DrawBox(leftX, stamY, leftX + 260, stamY + 12, GetColor(40, 40, 40), TRUE);
	DrawBox(leftX, stamY, leftX + stamW, stamY + 12, GetColor(200, 200, 50), TRUE);
	DrawFormatString(leftX + 270, stamY - 2, COLOR_WHITE, "STAMINA");
}

void UIManager::DrawTypingArea() const {
	const int bw = 800;
	const int bh = 100;
	const int bx = (SCREEN_WIDTH - bw) / 2;
	const int by = 80;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(bx, by, bx + bw, by + bh, GetColor(16, 16, 24), TRUE);
	DrawBox(bx, by, bx + bw, by + bh, GetColor(80, 120, 255), FALSE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	const int textX = bx + 24;
	const int textY = by + 16;

	DrawFormatString(textX, textY, COLOR_MAGENTA, "入力: %s", typingInput_.c_str());

	const std::string displayConverted = typingConverted_.empty() ? ConvertIfRomanji(ToLowerTrim(typingInput_)) : typingConverted_;
	DrawFormatString(textX, textY + 32, COLOR_GREEN, "変換: %s", displayConverted.c_str());

	const std::string prevConverted = typingPrevConverted_.empty() ? ConvertIfRomanji(ToLowerTrim(typingPrevInput_)) : typingPrevConverted_;
	DrawFormatString(textX, textY + 64, GetColor(150, 150, 150), "直前: %s (%s)", typingPrevInput_.c_str(), prevConverted.c_str());

	// 敵の詠唱窓
	const int castX = bx + 40;
	const int castY = by + bh;
	const int castW = bw - 80;
	const int castH = 60;

	if (!enemyCasting_.empty()) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
		DrawBox(castX, castY, castX + castW, castY + castH, GetColor(60, 0, 0), TRUE);
		DrawBox(castX, castY, castX + castW, castY + castH, GetColor(255, 60, 60), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		const size_t totalChars = GetUtf8CharCount(enemyCasting_);
		const float ratio = (enemyCastingWait_ > 0.0f) ? std::clamp(enemyCastingElapsed_ / enemyCastingWait_, 0.0f, 1.0f) : 0.0f;
		const size_t typedChars = static_cast<size_t>(ratio * totalChars);
		const std::string typedStr = Utf8Substring(enemyCasting_, typedChars);

		DrawFormatString(castX + 16, castY + 12, GetColor(120, 120, 120), "ENEMY: %s", enemyCasting_.c_str());
		//DrawFormatString(castX + 16, castY + 12, GetColor(255, 80, 80), "ENEMY: %s", typedStr.c_str());

		const int barW = static_cast<int>((castW - 32) * ratio);
		DrawBox(castX + 16, castY + 40, castX + castW - 16, castY + 46, GetColor(40, 0, 0), TRUE);
		DrawBox(castX + 16, castY + 40, castX + 16 + barW, castY + 46, GetColor(255, 200, 0), TRUE);
	}
	else if (enemyIsWaiting_ || enemyCooldown_ > 0.0f) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
		DrawBox(castX, castY, castX + castW, castY + 30, GetColor(30, 30, 30), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		if (enemyCooldown_ > 0.0f) {
			DrawFormatString(castX + 16, castY + 6, GetColor(180, 180, 180), "Enemy Cooldown... %.1fs", enemyCooldown_);
		}
		else {
			DrawFormatString(castX + 16, castY + 6, GetColor(180, 180, 180), "Enemy is waiting...");
		}
	}
}

void UIManager::DrawClashEvent() const {}

void UIManager::DrawPauseMenu() const {
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BLACK, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawString(450, 220, "ポーズメニュー", GetColor(255, 255, 0));
	DrawString(410, 280, (pauseCursor_ == 0 ? "→ " : "   "), COLOR_WHITE);
	DrawString(450, 280, "コマンド一覧", COLOR_WHITE);
	DrawString(410, 320, (pauseCursor_ == 1 ? "→ " : "   "), COLOR_WHITE);
	DrawString(450, 320, "タイトルに戻る", COLOR_WHITE);
	DrawString(410, 360, (pauseCursor_ == 2 ? "→ " : "   "), COLOR_WHITE);
	DrawString(450, 360, "ゲームを終了", COLOR_WHITE);

	if (pauseCursor_ == 0) {
		int y = 180;
		const int colW = 260;
		const int maxPerCol = 12;
		const int xBase = 800;

		DrawString(xBase, y, "【通常コマンド】", GetColor(200, 255, 200));
		y += 30;
		for (size_t i = 0; i < normalCommandList_.size(); ++i) {
			const int col = static_cast<int>(i) / maxPerCol;
			const int row = static_cast<int>(i) % maxPerCol;
			const int drawX = xBase + col * colW;
			const int drawY = y + row * 24;

			const std::string& orig = normalCommandList_[i];
			const std::string norm = ConvertIfRomanji(ToLowerTrim(orig));

			DrawString(drawX, drawY, orig.c_str(), GetColor(200, 200, 255));
			DrawFormatString(drawX + 130, drawY, GetColor(150, 255, 180), "(%s)", norm.c_str());
		}
		y += maxPerCol * 24 + 20;

		DrawString(xBase, y, "【必殺技コマンド】", GetColor(255, 200, 200));
		y += 30;
		for (size_t i = 0; i < ultimateCommandList_.size(); ++i) {
			const int col = static_cast<int>(i) / maxPerCol;
			const int row = static_cast<int>(i) % maxPerCol;
			const int drawX = xBase + col * colW;
			const int drawY = y + row * 24;

			const std::string& orig = ultimateCommandList_[i];
			const std::string norm = ConvertIfRomanji(ToLowerTrim(orig));

			DrawString(drawX, drawY, orig.c_str(), GetColor(255, 200, 200));
			DrawFormatString(drawX + 130, drawY, GetColor(255, 230, 160), "(%s)", norm.c_str());
		}
	}
}