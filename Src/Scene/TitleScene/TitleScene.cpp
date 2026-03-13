#include "TitleScene.h"

#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"
#include "../../Object/Attack/AttackManager.h"
#include "../../Common/UiManager.h"
#include "../../Common/RomanjiConverter.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip> // for setprecision

namespace {
	// =======================================================
	// ローマ字→ひらがな変換処理（揺れ吸収・正規化の要）
	// =======================================================
	static bool IsLikelyRomanji(const std::string& s) {
		if (s.empty()) return false;
		bool hasAlpha = false;
		for (unsigned char c : s) {
			// 全角文字（既に日本語化されている文字）が含まれていたら変換をキャンセル
			if (c >= 128) return false;

			// 半角英字が含まれているかチェック
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				hasAlpha = true;
			}
		}
		// 半角英字が1つでも含まれており、かつ全角文字が無いならローマ字とみなす
		return hasAlpha;
	}

	static std::string ConvertRomanjiToHiragana(const std::string& in) {
		RomanjiConverter conv;
		std::string filtered;

		// 空白は無視して連結しつつ、小文字化してまとめる
		for (unsigned char c : in) {
			if (c != ' ') {
				filtered += static_cast<char>(std::tolower(c));
			}
		}

		// 変換処理にすべて任せる
		return conv.convert(filtered);
	}

	static std::string ConvertIfRomanji(const std::string& s) {
		if (IsLikelyRomanji(s)) {
			std::string hira = ConvertRomanjiToHiragana(s);
			if (!hira.empty()) return hira;
		}
		return s;
	}

	// =======================================================
	// 例外クラッシュ対策：安全な文字列操作
	// =======================================================

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
}

TitleScene::TitleScene(void)
{
	handle_ = -1;
	attackManager_ = new AttackManager();
	keyInputHandle_ = -1;
	keyInputHandleCmd_ = -1;
	registeredDisplayRemaining_ = 0;
	registeredDisplayMessage_.clear();
	showCommandList_ = false;
	commandListScroll_ = 0;
	ignoreNextReturn_ = false;
	prevReturnDown_ = false;

	cmdHiraStr_.clear();
	inputHiraStr_.clear();
	combinedCommandList_.clear();
}

TitleScene::~TitleScene(void)
{
	if (keyInputHandle_ != -1) {
		DeleteKeyInput(keyInputHandle_);
		keyInputHandle_ = -1;
	}
	if (keyInputHandleCmd_ != -1) {
		DeleteKeyInput(keyInputHandleCmd_);
		keyInputHandleCmd_ = -1;
	}
	delete attackManager_;
	attackManager_ = nullptr; // 追加: 解放後の二次参照を防ぐ
}
void TitleScene::Init(void)
{
	keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
	SetActiveKeyInput(keyInputHandleCmd_);
	cmdInputBuf_[0] = '\0';
	registeredDisplayRemaining_ = 0;
	registeredDisplayMessage_.clear();
	showCommandList_ = false;
	combinedCommandList_.clear();
	commandListScroll_ = 0;
	ignoreNextReturn_ = false;
	prevReturnDown_ = false;
	cmdHiraStr_.clear();
	inputHiraStr_.clear();
}

void TitleScene::Load(void)
{
	handle_ = LoadGraph("Data/Image/Title2.png");
	LoadCommandsFromCSV("Data/CSV/Word.csv");
}

void TitleScene::LoadEnd(void)
{
	Init();
}

void TitleScene::LoadCommandsFromCSV(const std::string& path)
{
	commandMap_.clear();
	commandNames_.clear();

	std::ifstream file(path);
	if (!file.is_open()) {
		lastRegisteredCommand_ = "コマンドCSV読み込み失敗: " + path;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		std::istringstream iss(line);
		std::string name, type;
		if (std::getline(iss, name, ',') && std::getline(iss, type, ',')) {

			auto safe_trim = [](std::string& s) {
				s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !IsSpaceSafe(ch); }));
				s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !IsSpaceSafe(ch); }).base(), s.end());
				};
			safe_trim(name);
			safe_trim(type);

			if (name.empty()) continue;

			// ★ ここが要！CSVのコマンド名を「ひらがな」にして辞書に登録する
			std::string lowerName = ToLowerTrim(name);
			std::string hiraName = ConvertIfRomanji(lowerName);

			if (!hiraName.empty()) {
				commandMap_[hiraName] = type;
				commandNames_.push_back(name); // 表示用には元の文字列を保持しておく
			}
		}
	}
	lastRegisteredCommand_ = "コマンドCSV読み込み完了 (ひらがな正規化済)";
}

void TitleScene::ProcessTitleCommand(const std::string& rawInput)
{
	// 1. 入力を安全にトリム＆小文字化
	std::string inputTrim = ToLowerTrim(rawInput);
	if (inputTrim.empty()) {
		lastRegisteredCommand_ = "入力が空です";
		return;
	}

	// 2. ★入力をひらがなに変換（これが判定の共通言語になる）
	std::string inputHira = ConvertIfRomanji(inputTrim);

	// 3. ひらがな同士で辞書検索
	auto it = commandMap_.find(inputHira);

	// 見つからなければリセット
	if (it == commandMap_.end()) {
		lastRegisteredCommand_ = "未登録のコマンドです。再入力してください。";

		if (keyInputHandleCmd_ != -1) {
			DeleteKeyInput(keyInputHandleCmd_);
			keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandleCmd_);
		}
		cmdInputBuf_[0] = '\0';
		cmdHiraStr_.clear();
		return;
	}

	// 見つかった場合、CSV の type に従って処理する
	std::string rawType = it->second;
	rawType.erase(rawType.begin(), std::find_if(rawType.begin(), rawType.end(), [](unsigned char ch) { return !IsSpaceSafe(ch); }));
	rawType.erase(std::find_if(rawType.rbegin(), rawType.rend(), [](unsigned char ch) { return !IsSpaceSafe(ch); }).base(), rawType.end());

	auto colon = rawType.find(':');
	std::string type = (colon != std::string::npos) ? rawType.substr(0, colon) : rawType;
	std::string typeLower = ToLowerTrim(type);

	static const std::vector<std::string> REGISTER_KEYS = { "register", "ultimate" };
	static const std::vector<std::string> START_KEYS = { "start", "play", "game" };
	static const std::vector<std::string> LIST_KEYS = { "list", "commands", "help" };
	static const std::vector<std::string> EXIT_KEYS = { "exit", "quit", "end" };

	auto isOneOf = [&](const std::string& v, const std::vector<std::string>& opts)->bool {
		for (const auto& o : opts) if (v == o) return true;
		return false;
		};

	if (isOneOf(typeLower, REGISTER_KEYS)) {
		if (keyInputHandleCmd_ != -1) {
			DeleteKeyInput(keyInputHandleCmd_);
			keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			cmdInputBuf_[0] = '\0';
			cmdHiraStr_.clear();
		}
		isRegisteringUltimate_ = true;
		keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
		SetActiveKeyInput(keyInputHandle_);
		inputBuf_[0] = '\0';
		inputHiraStr_.clear();
		lastRegisteredCommand_ = std::string("必殺技登録モード開始: ") + rawInput;
		return;
	}

	if (isOneOf(typeLower, START_KEYS)) {
		lastRegisteredCommand_ = std::string("ゲーム開始コマンド: ") + rawInput;
		SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
		return;
	}

	if (isOneOf(typeLower, LIST_KEYS)) {
		combinedCommandList_.clear();
		for (const auto& n : commandNames_) {
			combinedCommandList_.push_back(n);
		}
		if (attackManager_) {
			for (const auto& p : attackManager_->registeredCommands_) {
				combinedCommandList_.push_back(p.first + " (必殺)");
			}
		}
		if (combinedCommandList_.empty()) {
			lastRegisteredCommand_ = "コマンド一覧は空です";
			showCommandList_ = false;
		}
		else {
			showCommandList_ = true;
			commandListScroll_ = 0;
			ignoreNextReturn_ = true;
			lastRegisteredCommand_ = "コマンド一覧表示中";
		}
		return;
	}

	if (isOneOf(typeLower, EXIT_KEYS)) {
		lastRegisteredCommand_ = "終了コマンドによる終了";
		DxLib_End();
		exit(0);
		return;
	}

	lastRegisteredCommand_ = std::string("コマンド検出: ") + rawInput + " -> " + rawType;
}

void TitleScene::Update(void)
{
	// 登録メッセージの表示時間管理
	if (registeredDisplayRemaining_ > 0) {
		--registeredDisplayRemaining_;
		if (registeredDisplayRemaining_ == 0) {
			registeredDisplayMessage_.clear();
		}
	}

	bool curReturnDown = (CheckHitKey(KEY_INPUT_RETURN) != 0);
	bool newReturnPress = (curReturnDown && !prevReturnDown_);

	// --- コマンド一覧表示モード ---
	if (showCommandList_) {
		if (CheckHitKey(KEY_INPUT_UP)) {
			if (commandListScroll_ > 0) --commandListScroll_;
		}
		if (CheckHitKey(KEY_INPUT_DOWN)) {
			int maxScroll = (int)combinedCommandList_.size() - kCommandListPageSize;
			if (maxScroll < 0) maxScroll = 0;
			if (commandListScroll_ < maxScroll) ++commandListScroll_;
		}

		if (ignoreNextReturn_ && newReturnPress) {
			ignoreNextReturn_ = false;
		}
		else if (newReturnPress || InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE)) {
			showCommandList_ = false;
			combinedCommandList_.clear();
			commandListScroll_ = 0;

			if (keyInputHandleCmd_ != -1) {
				DeleteKeyInput(keyInputHandleCmd_);
				keyInputHandleCmd_ = -1;
			}
			keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandleCmd_);

			cmdInputBuf_[0] = '\0';
			lastRegisteredCommand_.clear();
			registeredDisplayMessage_.clear();
			registeredDisplayRemaining_ = 0;

			prevReturnDown_ = curReturnDown;
			return;
		}
		prevReturnDown_ = curReturnDown;
		return;
	}

	// --- 必殺技コマンド登録モード ---
	if (!isRegisteringUltimate_) {
		if (CheckHitKey(KEY_INPUT_F1)) {
			if (keyInputHandleCmd_ != -1) {
				DeleteKeyInput(keyInputHandleCmd_);
				keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
				cmdInputBuf_[0] = '\0';
			}
			isRegisteringUltimate_ = true;
			keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandle_);
			inputBuf_[0] = '\0';
			inputHiraStr_.clear();
		}
	}
	else {
		GetKeyInputString(inputBuf_, keyInputHandle_);
		inputHiraStr_ = std::string(inputBuf_);

		if (CheckKeyInput(keyInputHandle_) == 1) {
			DeleteKeyInput(keyInputHandle_);
			if (keyInputHandleCmd_ == -1) {
				keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			}
			SetActiveKeyInput(keyInputHandleCmd_);
			keyInputHandle_ = -1;
			isRegisteringUltimate_ = false;

			if (attackManager_ && inputHiraStr_.size() != 0) {
				// 登録する必殺技名もひらがなに正規化してからAttackManagerに渡す
				std::string commandStr = ConvertIfRomanji(inputHiraStr_);

				std::string commandId = attackManager_->RegisterUltimateCommand(commandStr, 5);
				attackManager_->ReloadCommands();

				if (!commandId.empty()) {
					auto it = attackManager_->ultimateCommandDataMap_.find(commandId);
					if (it != attackManager_->ultimateCommandDataMap_.end()) {
						int dmg = it->second.damage;
						float spd = it->second.speed;
						std::ostringstream oss;
						oss << "登録しました: " << commandStr << " (ID:" << commandId
							<< " DMG:" << dmg << " SPD:" << std::fixed << std::setprecision(1) << spd << ")";
						registeredDisplayMessage_ = oss.str();
					}
					else {
						registeredDisplayMessage_ = std::string("登録しました: ") + commandStr + " (ID:" + commandId + ")";
					}
					registeredDisplayRemaining_ = kRegisteredDisplayFrames;
					lastRegisteredCommand_ = registeredDisplayMessage_;
				}
				else {
					lastRegisteredCommand_ = "登録に失敗しました";
				}
			}
			else {
				lastRegisteredCommand_ = "登録に失敗しました";
			}

			inputBuf_[0] = '\0';
			inputHiraStr_.clear();
			cmdInputBuf_[0] = '\0';
			SetActiveKeyInput(keyInputHandleCmd_);
		}

		if (CheckHitKey(KEY_INPUT_ESCAPE)) {
			DeleteKeyInput(keyInputHandle_);
			if (keyInputHandleCmd_ == -1) {
				keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			}
			SetActiveKeyInput(keyInputHandleCmd_);
			keyInputHandle_ = -1;
			isRegisteringUltimate_ = false;
			inputBuf_[0] = '\0';
			inputHiraStr_.clear();
			cmdInputBuf_[0] = '\0';
		}
		prevReturnDown_ = curReturnDown;
		return;
	}

	// --- メイン入力窓（常時） ---
	if (keyInputHandleCmd_ != -1) {
		GetKeyInputString(cmdInputBuf_, keyInputHandleCmd_);
		cmdHiraStr_ = std::string(cmdInputBuf_);

		if (CheckKeyInput(keyInputHandleCmd_) == 1) {
			// 1. 入力された文字列を退避
			std::string raw(cmdInputBuf_);

			// 2. ★自分が生きているうちに後片付けを終わらせる（例外対策）
			if (!isRegisteringUltimate_) {
				cmdInputBuf_[0] = '\0';
				cmdHiraStr_.clear();
				SetActiveKeyInput(keyInputHandleCmd_);
			}
			else {
				cmdInputBuf_[0] = '\0';
				cmdHiraStr_.clear();
			}

			// 3. コマンド処理を実行（ここでシーンが切り替わり、自分が消滅する可能性がある）
			ProcessTitleCommand(raw);

			// 4. ★自分が消滅したかもしれないので、これ以上はメンバ変数に触らずに即リターン！
			return;
		}
	}

	// 必殺技リストのスクロール操作
	if (!isRegisteringUltimate_ && attackManager_) {
		int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
		if (CheckHitKey(KEY_INPUT_UP)) {
			if (scrollOffset_ > 0) scrollOffset_--;
		}
		if (CheckHitKey(KEY_INPUT_DOWN)) {
			if (scrollOffset_ < listSize - 1) scrollOffset_++;
		}
	}

	// スペースキーによるゲーム開始
	if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);

		// ★ここも例外対策！シーン遷移したら即リターンする
		return;
	}

	// シーンが切り替わらなかった場合のみ、ここに到達して変数を更新する
	prevReturnDown_ = curReturnDown;
}
void TitleScene::Draw(void)
{
	SetBackgroundColor(0, 0, 0);

	int screenW = 0;
	int screenH = 0;
	GetDrawScreenSize(&screenW, &screenH);

	const int inputBoxW = 800;
	const int inputBoxH = 32;
	int candidateX = screenW / 2 - inputBoxW / 2;
	int inputX = (candidateX > 0) ? candidateX : 0;
	int inputY = screenH / 2 - inputBoxH / 2;

	DrawString(inputX, inputY - 56, "入力窓: 文言を入力してEnter（CSV参照）", GetColor(255, 255, 255));

	if (keyInputHandleCmd_ != -1) {
		DrawKeyInputString(inputX, inputY, keyInputHandleCmd_);
		std::string displayStr = ConvertIfRomanji(std::string(cmdInputBuf_));
		DrawFormatString(inputX, inputY + 40, GetColor(0, 255, 0), "入力: %s", displayStr.c_str());

		if (!cmdHiraStr_.empty()) {
			std::string displayHira = ConvertIfRomanji(cmdHiraStr_);
			DrawFormatString(inputX, inputY + 64, GetColor(0, 200, 255), "ひらがな: %s", displayHira.c_str());
		}
	}

	if (isRegisteringUltimate_) {
		DrawString(inputX, inputY - 56, "必殺技登録モード: 名前を入力してEnter（Escでキャンセル）", GetColor(255, 255, 255));
		if (keyInputHandle_ != -1) {
			DrawKeyInputString(inputX, inputY, keyInputHandle_);
		}

		std::string displayRegStr = ConvertIfRomanji(std::string(inputBuf_));
		DrawFormatString(inputX, inputY + 40, GetColor(0, 255, 0), "登録用入力: %s", displayRegStr.c_str());

		if (!inputHiraStr_.empty()) {
			std::string displayRegHira = ConvertIfRomanji(inputHiraStr_);
			DrawFormatString(inputX, inputY + 64, GetColor(0, 200, 255), "登録用(ひらがな): %s", displayRegHira.c_str());
		}
	}
	else {
		DrawString(inputX, inputY + 80, "F1で必殺技登録モード  SPACEでゲーム開始", GetColor(255, 255, 255));
		const std::string& statusMsg = (!registeredDisplayMessage_.empty()) ? registeredDisplayMessage_ : lastRegisteredCommand_;
		if (!statusMsg.empty()) {
			DrawFormatString(inputX, inputY + 112, GetColor(255, 255, 0), "状態: %s", ConvertIfRomanji(statusMsg).c_str());
		}
	}

	if (showCommandList_) {
		int w = 700;
		int h = 500;
		int x0 = screenW / 2 - w / 2;
		int y0 = screenH / 2 - h / 2;
		int x1 = x0 + w;
		int y1 = y0 + h;
		DrawBox(x0, y0, x1, y1, GetColor(20, 20, 20), TRUE);
		DrawBox(x0 + 2, y0 + 2, x1 - 2, y1 - 2, GetColor(80, 80, 80), FALSE);
		DrawFormatString(x0 + 12, y0 + 8, GetColor(255, 255, 255), "コマンド一覧 (Enter/Spaceで戻る) - 通常コマンド と 必殺技コマンド");

		int lineH = 24;
		int startIdx = commandListScroll_;
		int maxDisplay = kCommandListPageSize;
		for (int i = 0; i < maxDisplay; ++i) {
			int idx = startIdx + i;
			if (idx >= (int)combinedCommandList_.size()) break;
			DrawFormatString(x0 + 16, y0 + 40 + i * lineH, GetColor(200, 200, 200), "%3d: %s", idx + 1, ConvertIfRomanji(combinedCommandList_[idx]).c_str());
		}
		if ((int)combinedCommandList_.size() > maxDisplay) {
			DrawFormatString(x0 + 16, y1 - 28, GetColor(180, 180, 180), "↑↓でスクロール (%d/%d)", startIdx + 1, (int)combinedCommandList_.size());
		}
		return;
	}

	DrawString(50, 800, "このバトルタイピングは、コマンドをtypingして、回避や移動、攻撃、必殺、などを繰り出しボスを倒すのが目的です\nコマンドは、みぎ、ひだり、うえ、した、みぎにいどう、などいろいろあります\nコマンドはゲームシーンでTAB押下で確認することができます\n攻撃コマンドもこうげき、はっしゃなど複数コマンドがあります\n必殺技は各々が命名して生成することができます(6文字以上)。技によってステータスが変わります。強い必殺技やかっこいい必殺技を生成しましょう", GetColor(255, 255, 255));

	if (attackManager_) {
		int candidateRightX = screenW - 700;
		int x = (candidateRightX > 0) ? candidateRightX : 0;
		int y = 100;
		int lineHeight = 32;
		int maxDisplay = 20;
		int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
		DrawString(x, 80, "必殺技一覧", GetColor(255, 255, 0));
		for (int i = 0; i < maxDisplay; ++i) {
			int idx = i + scrollOffset_;
			if (idx >= listSize) break;
			const auto& pair = attackManager_->registeredCommands_[idx];
			const std::string& commandId = pair.second;
			int damage = 0;
			auto it = attackManager_->ultimateCommandDataMap_.find(commandId);
			if (it != attackManager_->ultimateCommandDataMap_.end()) {
				damage = it->second.damage;
			}
			DrawFormatString(x, y + i * lineHeight, GetColor(255, 255, 0),
				"%2d: %s [DMG:%d]", idx + 1, ConvertIfRomanji(pair.first).c_str(), damage);
		}
		if (listSize > maxDisplay) {
			DrawString(x, y + maxDisplay * lineHeight, "↑↓でスクロール", GetColor(200, 200, 200));
		}
	}

	if (isPause_) {
		UIManager::GetInstance().Draw(UIManager::UIState::Pause);
		return;
	}
}

void TitleScene::Release(void)
{
	if (handle_ != -1) {
		DeleteGraph(handle_);
		handle_ = -1; // 追加: 二重解放防止
	}
	if (keyInputHandle_ != -1) {
		DeleteKeyInput(keyInputHandle_);
		keyInputHandle_ = -1;
	}
	if (keyInputHandleCmd_ != -1) {
		DeleteKeyInput(keyInputHandleCmd_);
		keyInputHandleCmd_ = -1;
	}
}