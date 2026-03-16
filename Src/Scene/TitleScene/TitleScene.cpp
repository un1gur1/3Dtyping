
#include "TitleScene.h"
#include <DxLib.h>
#include "EffekseerForDXLib.h"
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
#include <iomanip>
#include <random> // 追加

// =======================================================
// 文字列・正規化ヘルパー（無名名前空間）
// =======================================================
namespace {
	// ... 既存ヘルパーは省略せずそのまま ...
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

	static bool IsSpaceSafe(const unsigned char ch) {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}

	static std::string ToLowerTrim(const std::string& s) {
		std::string t = s;
		t.erase(t.begin(), std::find_if(t.begin(), t.end(), [](unsigned char ch) { return !IsSpaceSafe(ch); }));
		t.erase(std::find_if(t.rbegin(), t.rend(), [](unsigned char ch) { return !IsSpaceSafe(ch); }).base(), t.end());
		for (char& c : t) {
			const unsigned char uc = static_cast<unsigned char>(c);
			if (uc < 128) c = static_cast<char>(std::tolower(c));
		}
		return t;
	}
}

// =======================================================
// 初期化・ロード・破棄
// =======================================================
TitleScene::TitleScene(void) {
	attackManager_ = new AttackManager();
	// RNG の初期化はコンストラクタでも可（安全のため空シード）
	efkRng_.seed(static_cast<unsigned int>(std::random_device{}()));
}

TitleScene::~TitleScene(void) {
	// Release() は Scene 側で呼ばれる前提だが念のため
	Release();

	if (attackManager_) {
		delete attackManager_;
		attackManager_ = nullptr;
	}
}

void TitleScene::Init(void) {
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

	// Effekseer の表示サイズ同期（ウィンドウサイズが変わる可能性があるため here でも呼べる）
	int sw = 0, sh = 0;
	GetDrawScreenSize(&sw, &sh);
	if (effekseerInitialized_) {
		Effekseer_Set2DSetting(sw, sh);
	}

	// 花火初期値
	efkSpawnTimer_ = 0.0f;
	efkSpawnInterval_ = 0.5f; // 基本間隔（秒）
	efkMaxSimultaneous_ = 6;
}

void TitleScene::Load(void) {
	handle_ = LoadGraph("Data/Image/Title2.png");
	titleHandle_ = LoadGraph("Data/Image/title.png");
	LoadCommandsFromCSV("Data/CSV/Word.csv");

	// --- Effekseer 初期化 ---
	if (!effekseerInitialized_) {
		// 適切なパーティクル上限を指定
		if (Effekseer_Init(2000) == 0) {
			effekseerInitialized_ = true;
		}
	}

	// ウィンドウサイズを Effekseer に通知
	int sw = 0, sh = 0;
	GetDrawScreenSize(&sw, &sh);
	if (effekseerInitialized_) {
		Effekseer_Set2DSetting(sw, sh);
	}

	// efk ファイルの読み込み（相対パス）
	const char* efkPath = "Data/Image/efe/efe.efk";
	if (effekseerInitialized_) {
		efkResourceHandle_ = LoadEffekseerEffect(efkPath, 1.0f);
		if (efkResourceHandle_ != -1) {
			// 1個目は中央上に固定で再生（見本）
			int sw2 = 0, sh2 = 0;
			GetDrawScreenSize(&sw2, &sh2);
			int h = PlayEffekseer2DEffect(efkResourceHandle_);
			if (h != -1) {
				SetPosPlayingEffekseer2DEffect(h, static_cast<float>(sw2) * 0.5f, static_cast<float>(sh2) * 0.25f, 0.0f);
				SetScalePlayingEffekseer2DEffect(h, 3.0f, 3.0f, 3.0f);
				efkPlayingHandles_.push_back(h);
			}
		}
	}
}

void TitleScene::LoadEnd(void) {
	Init();
}

void TitleScene::Release(void) {
	// 画像リソースとキー入力ハンドルの解放
	if (handle_ != -1) {
		DeleteGraph(handle_);
		handle_ = -1;
	}
	if (titleHandle_ != -1) {
		DeleteGraph(titleHandle_);
		titleHandle_ = -1;
	}
	if (keyInputHandle_ != -1) {
		DeleteKeyInput(keyInputHandle_);
		keyInputHandle_ = -1;
	}
	if (keyInputHandleCmd_ != -1) {
		DeleteKeyInput(keyInputHandleCmd_);
		keyInputHandleCmd_ = -1;
	}

	
	if (!efkPlayingHandles_.empty()) {
		std::vector<int> alive;
		alive.reserve(efkPlayingHandles_.size());

		for (int ph : efkPlayingHandles_) {
			// 再生中(TRUE = 1)ならリストに残す
			if (IsEffekseer2DEffectPlaying(ph) == TRUE) {
				alive.push_back(ph);
			}
			// 再生終了(FALSE = 0)なら何もしない（自然消滅させる）
		}

		efkPlayingHandles_.swap(alive);
	}
	if (effekseerInitialized_) {
		Effkseer_End();
		effekseerInitialized_ = false;
	}
}

// =======================================================
// CSV読み込み・コマンド処理
// =======================================================
void TitleScene::LoadCommandsFromCSV(const std::string& path) {
	// 既存実装そのまま
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
				s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'); }));
				s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'); }).base(), s.end());
				};
			safe_trim(name);
			safe_trim(type);

			if (name.empty()) continue;

			const std::string lowerName = ToLowerTrim(name);
			const std::string hiraName = ConvertIfRomanji(lowerName);

			if (!hiraName.empty()) {
				commandMap_[hiraName] = type;
				commandNames_.push_back(name);
			}
		}
	}
	lastRegisteredCommand_ = "コマンド読み込み完了";
}

void TitleScene::ProcessTitleCommand(const std::string& rawInput) {
	// 既存実装そのまま
	const std::string inputTrim = ToLowerTrim(rawInput);
	if (inputTrim.empty()) {
		lastRegisteredCommand_ = "入力が空です";
		return;
	}

	const std::string inputHira = ConvertIfRomanji(inputTrim);
	const auto it = commandMap_.find(inputHira);

	if (it == commandMap_.end()) {
		lastRegisteredCommand_ = "未登録のコマンドです";
		if (keyInputHandleCmd_ != -1) {
			DeleteKeyInput(keyInputHandleCmd_);
			keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandleCmd_);
		}
		cmdInputBuf_[0] = '\0';
		cmdHiraStr_.clear();
		return;
	}

	std::string rawType = it->second;
	rawType.erase(rawType.begin(), std::find_if(rawType.begin(), rawType.end(), [](unsigned char ch) { return !(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'); }));
	rawType.erase(std::find_if(rawType.rbegin(), rawType.rend(), [](unsigned char ch) { return !(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'); }).base(), rawType.end());

	const auto colon = rawType.find(':');
	const std::string type = (colon != std::string::npos) ? rawType.substr(0, colon) : rawType;
	const std::string typeLower = ToLowerTrim(type);

	static const std::vector<std::string> REGISTER_KEYS = { "register", "ultimate" };
	static const std::vector<std::string> START_KEYS = { "start", "play", "game" };
	static const std::vector<std::string> LIST_KEYS = { "list", "commands", "help" };
	static const std::vector<std::string> EXIT_KEYS = { "exit", "quit", "end" };

	auto isOneOf = [&](const std::string& v, const std::vector<std::string>& opts) -> bool {
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
		lastRegisteredCommand_ = std::string("登録モード: ") + rawInput;
		return;
	}

	if (isOneOf(typeLower, START_KEYS)) {
		lastRegisteredCommand_ = std::string("開始: ") + rawInput;
		SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
		return;
	}

	if (isOneOf(typeLower, LIST_KEYS)) {
		combinedCommandList_.clear();
		for (const auto& n : commandNames_) combinedCommandList_.push_back(n);
		if (attackManager_) {
			for (const auto& p : attackManager_->registeredCommands_) combinedCommandList_.push_back(p.first + " (必殺)");
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
		DxLib_End();
		exit(0);
		return;
	}

	lastRegisteredCommand_ = std::string("検出: ") + rawInput;
}

// =======================================================
// Update
// =======================================================
void TitleScene::Update(void) {
	// Effekseer 更新（2D）
	if (effekseerInitialized_) {
		UpdateEffekseer2D();
	}

	// ----- 花火（ランダムスポーン）処理 -----
	// 1/60 フレーム想定
	const float frameDt = 1.0f / 60.0f;

	// 再生終了したハンドルを掃除
	if (!efkPlayingHandles_.empty()) {
		std::vector<int> alive;
		alive.reserve(efkPlayingHandles_.size());
		for (int ph : efkPlayingHandles_) {
			// IsEffekseer2DEffectPlaying: 0 -> 再生中, -1 -> 再生終了/無効
			int playing = IsEffekseer2DEffectPlaying(ph);
			if (playing == 0) {
				alive.push_back(ph);
			}
			else {
				// 明示的に停止しておく（必要なら）
				StopEffekseer2DEffect(ph);
			}
		}
		efkPlayingHandles_.swap(alive);
	}

	// スポーンタイマー更新
	efkSpawnTimer_ -= frameDt;
	if (efkSpawnTimer_ <= 0.0f) {
		// 同時再生上限を越えていなければスポーン
		if (effekseerInitialized_ && efkResourceHandle_ != -1 && static_cast<int>(efkPlayingHandles_.size()) < efkMaxSimultaneous_) {
			int sw = 0, sh = 0;
			GetDrawScreenSize(&sw, &sh);
			if (sw > 0 && sh > 0) {
				// ランダム位置（画面幅全体、高さは上部〜中段に限定して花火っぽく）
				std::uniform_real_distribution<float> dx(0.1f * sw, 0.9f * sw);
				std::uniform_real_distribution<float> dy(0.05f * sh, 0.5f * sh);
				std::uniform_real_distribution<float> dscale(1.0f, 3.5f);
				std::uniform_int_distribution<int> drgb(120, 255);

				float px = dx(efkRng_);
				float py = dy(efkRng_);
				float scale = dscale(efkRng_);
				int r = drgb(efkRng_);
				int g = drgb(efkRng_);
				int b = drgb(efkRng_);

				int ph = PlayEffekseer2DEffect(efkResourceHandle_);
				if (ph != -1) {
					SetPosPlayingEffekseer2DEffect(ph, px, py, 0.0f);
					SetScalePlayingEffekseer2DEffect(ph, scale, scale, scale);
					// 色をランダムに（透過255）
					SetColorPlayingEffekseer2DEffect(ph, r, g, b, 255);
					efkPlayingHandles_.push_back(ph);
				}
			}
		}
		// 次回スポーンまでの間隔をランダム化して少しばらつかせる
		std::uniform_real_distribution<float> jitter(0.6f * efkSpawnInterval_, 1.4f * efkSpawnInterval_);
		efkSpawnTimer_ = jitter(efkRng_);
	}
	// ----- 花火処理 終了 -----

	if (registeredDisplayRemaining_ > 0) {
		--registeredDisplayRemaining_;
		if (registeredDisplayRemaining_ == 0) {
			registeredDisplayMessage_.clear();
		}
	}

	const bool curReturnDown = (CheckHitKey(KEY_INPUT_RETURN) != 0);
	const bool newReturnPress = (curReturnDown && !prevReturnDown_);

	// --- 1. コマンド一覧表示モード ---
	if (showCommandList_) {
		if (CheckHitKey(KEY_INPUT_UP) && commandListScroll_ > 0) --commandListScroll_;
		if (CheckHitKey(KEY_INPUT_DOWN)) {
			int maxScroll = static_cast<int>(combinedCommandList_.size()) - COMMAND_LIST_PAGE_SIZE;
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
			}
			keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandleCmd_);
			cmdInputBuf_[0] = '\0';

			prevReturnDown_ = curReturnDown;
			return;
		}
		prevReturnDown_ = curReturnDown;
		return;
	}

	// --- 2. 必殺技コマンド登録モード ---
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
			if (keyInputHandleCmd_ == -1) keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandleCmd_);
			keyInputHandle_ = -1;
			isRegisteringUltimate_ = false;

			if (attackManager_ && !inputHiraStr_.empty()) {
				const std::string commandStr = ConvertIfRomanji(inputHiraStr_);
				const std::string commandId = attackManager_->RegisterUltimateCommand(commandStr, 5);
				attackManager_->ReloadCommands();

				if (!commandId.empty()) {
					const auto it = attackManager_->ultimateCommandDataMap_.find(commandId);
					if (it != attackManager_->ultimateCommandDataMap_.end()) {
						std::ostringstream oss;
						oss << "登録成功: " << commandStr << " (DMG:" << it->second.damage << ")";
						registeredDisplayMessage_ = oss.str();
					}
					else {
						registeredDisplayMessage_ = "登録成功: " + commandStr;
					}
					registeredDisplayRemaining_ = REGISTERED_DISPLAY_FRAMES;
					lastRegisteredCommand_ = registeredDisplayMessage_;
				}
				else {
					lastRegisteredCommand_ = "登録失敗 (文字数不足または重複)";
				}
			}

			inputBuf_[0] = '\0';
			inputHiraStr_.clear();
			cmdInputBuf_[0] = '\0';
			SetActiveKeyInput(keyInputHandleCmd_);
		}

		if (CheckHitKey(KEY_INPUT_ESCAPE)) {
			DeleteKeyInput(keyInputHandle_);
			if (keyInputHandleCmd_ == -1) keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
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

	// --- 3. メイン入力窓（常時） ---
	if (keyInputHandleCmd_ != -1) {
		GetKeyInputString(cmdInputBuf_, keyInputHandleCmd_);
		cmdHiraStr_ = std::string(cmdInputBuf_);

		if (CheckKeyInput(keyInputHandleCmd_) == 1) {
			const std::string raw(cmdInputBuf_);

			cmdInputBuf_[0] = '\0';
			cmdHiraStr_.clear();
			if (!isRegisteringUltimate_) SetActiveKeyInput(keyInputHandleCmd_);

			ProcessTitleCommand(raw);
			return; // 遷移する可能性があるので即リターン
		}
	}

	// 必殺技リストのスクロール操作
	if (!isRegisteringUltimate_ && attackManager_) {
		const int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
		if (CheckHitKey(KEY_INPUT_UP) && scrollOffset_ > 0) scrollOffset_--;
		if (CheckHitKey(KEY_INPUT_DOWN) && scrollOffset_ < listSize - 1) scrollOffset_++;
	}

	// SPACEによるゲーム開始
	if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
		return;
	}

	prevReturnDown_ = curReturnDown;
}

// =======================================================
// Draw
// =======================================================
void TitleScene::Draw(void) {
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	// 1. 背景描画 (Title2.png があれば画面サイズに引き伸ばして描画)
	if (handle_ != -1) {
		DrawExtendGraph(0, 0, screenW, screenH, handle_, TRUE);
	}
	else {
		SetBackgroundColor(20, 20, 30);
	}

	// 背景を少し暗くしてUIを見やすくする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
	DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// タイトルロゴ描画 (Title.png があれば中央上部に表示)
	if (titleHandle_ != -1) {
		int gw = 0, gh = 0;
		GetGraphSize(titleHandle_, &gw, &gh);
		const float scale = 0.3f; // 必要なら調整
		const int logoCenterX = screenW / 2; // 横は画面中央
		const int topMargin = 20; // 上からの余白
		const int logoCenterY = topMargin + static_cast<int>(gh * scale * 0.5f);
		DrawRotaGraph(logoCenterX, logoCenterY, scale, 0.0, titleHandle_, TRUE);
	}

	// --- 定数定義 ---
	const unsigned int colWhite = GetColor(255, 255, 255);
	const unsigned int colGreen = GetColor(100, 255, 100);
	const unsigned int colCyan = GetColor(100, 200, 255);
	const unsigned int colYellow = GetColor(255, 255, 100);

	// 2. 中央の入力エリア
	const int inputW = 700;
	const int inputH = 120;
	const int inputX = (screenW - inputW) / 2;
	const int inputY = screenH / 2 - 80;

	// 入力窓の背景パネル
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(inputX, inputY, inputX + inputW, inputY + inputH, GetColor(20, 20, 30), TRUE);
	DrawBox(inputX, inputY, inputX + inputW, inputY + inputH, GetColor(100, 150, 255), FALSE); // 青枠
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if (isRegisteringUltimate_) {
		DrawString(inputX + 20, inputY - 30, "【必殺技登録モード】 名前を入力してEnter (Escでキャンセル)", colYellow);

		if (keyInputHandle_ != -1) DrawKeyInputString(inputX + 20, inputY + 20, keyInputHandle_);

		const std::string displayRegStr = ConvertIfRomanji(std::string(inputBuf_));
		DrawFormatString(inputX + 20, inputY + 50, colGreen, "入力: %s", displayRegStr.c_str());
		if (!inputHiraStr_.empty()) {
			const std::string displayRegHira = ConvertIfRomanji(inputHiraStr_);
			DrawFormatString(inputX + 20, inputY + 80, colCyan, "変換: %s", displayRegHira.c_str());
		}
	}
	else {
		DrawString(inputX + 20, inputY - 30, "【コマンド入力】 かいし, いちらん, とうろく などを入力してEnter", colWhite);

		if (keyInputHandleCmd_ != -1) DrawKeyInputString(inputX + 20, inputY + 20, keyInputHandleCmd_);

		const std::string displayStr = ConvertIfRomanji(std::string(cmdInputBuf_));
		DrawFormatString(inputX + 20, inputY + 50, colGreen, "入力: %s", displayStr.c_str());
		if (!cmdHiraStr_.empty()) {
			const std::string displayHira = ConvertIfRomanji(cmdHiraStr_);
			DrawFormatString(inputX + 20, inputY + 80, colCyan, "変換: %s", displayHira.c_str());
		}
	}

	// 3. ステータス・操作説明 (入力窓の少し下)
	const std::string statusMsg = (!registeredDisplayMessage_.empty()) ? registeredDisplayMessage_ : lastRegisteredCommand_;
	if (!statusMsg.empty()) {
		DrawFormatString(inputX + 20, inputY + inputH + 20, colYellow, "Status: %s", ConvertIfRomanji(statusMsg).c_str());
	}
	DrawString(inputX + 20, inputY + inputH + 50, "開始系の言葉を入力でゲーム開始", colWhite);

	// 4. 必殺技一覧 (右側パネル)
	if (attackManager_) {
		const int listW = 350;
		const int listH = 400;
		const int listX = screenW - listW - 40;
		const int listY = 80;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
		DrawBox(listX, listY, listX + listW, listY + listH, GetColor(30, 20, 20), TRUE);
		DrawBox(listX, listY, listX + listW, listY + listH, GetColor(255, 100, 100), FALSE); // 赤枠
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		DrawString(listX + 20, listY + 15, "◆ 登録済み必殺技", colYellow);
		DrawLine(listX + 10, listY + 40, listX + listW - 10, listY + 40, colYellow);

		const int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
		const int maxDisplay = 12;
		const int lineHeight = 26;

		for (int i = 0; i < maxDisplay; ++i) {
			const int idx = i + scrollOffset_;
			if (idx >= listSize) break;

			const auto& pair = attackManager_->registeredCommands_[idx];
			int damage = 0;
			const auto it = attackManager_->ultimateCommandDataMap_.find(pair.second);
			if (it != attackManager_->ultimateCommandDataMap_.end()) damage = it->second.damage;

			DrawFormatString(listX + 20, listY + 55 + i * lineHeight, colWhite, "%2d: %s [DMG:%d]", idx + 1, ConvertIfRomanji(pair.first).c_str(), damage);
		}
		if (listSize > maxDisplay) {
			DrawString(listX + 20, listY + listH - 25, "↑↓キーでスクロール", GetColor(150, 150, 150));
		}
	}

	// 5. ゲームの目的・説明 (画面下部パネル)
	const int descW = screenW - 80;
	const int descH = 120;
	const int descX = 40;
	const int descY = screenH - descH - 40;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
	DrawBox(descX, descY, descX + descW, descY + descH, GetColor(10, 10, 10), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawString(descX + 20, descY + 20, "【遊び方】", colCyan);
	DrawString(descX + 20, descY + 50, "コマンドをタイピングして、移動・回避・攻撃を繰り出しボスを倒せ！", colWhite);
	DrawString(descX + 20, descY + 75, "・ゲーム中に[TAB]キーでコマンド一覧を確認可能", colWhite);
	DrawString(descX + 20, descY + 100, "とうろくと入力し自分だけのオリジナル必殺技(6文字以上推奨)を登録しよう！", colWhite);

	// 6. コマンド一覧の全画面表示 (list コマンド時)
	if (showCommandList_) {
		const int w = 800;
		const int h = 600;
		const int x0 = (screenW - w) / 2;
		const int y0 = (screenH - h) / 2;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
		DrawBox(x0, y0, x0 + w, y0 + h, GetColor(10, 10, 20), TRUE);
		DrawBox(x0, y0, x0 + w, y0 + h, colCyan, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		DrawString(x0 + 20, y0 + 20, "◆ コマンド一覧 (Enter / Space で戻る)", colYellow);
		DrawLine(x0 + 10, y0 + 50, x0 + w - 10, y0 + 50, colCyan);

		const int startIdx = commandListScroll_;
		for (int i = 0; i < COMMAND_LIST_PAGE_SIZE; ++i) {
			const int idx = startIdx + i;
			if (idx >= static_cast<int>(combinedCommandList_.size())) break;
			DrawFormatString(x0 + 30, y0 + 70 + i * 24, colWhite, "%3d: %s", idx + 1, ConvertIfRomanji(combinedCommandList_[idx]).c_str());
		}
		if (combinedCommandList_.size() > COMMAND_LIST_PAGE_SIZE) {
			DrawFormatString(x0 + 30, y0 + h - 30, GetColor(150, 150, 150), "↑↓でスクロール (%d/%d)", startIdx + 1, combinedCommandList_.size());
		}
	}

	// Effekseer 全体描画（2D）
	if (effekseerInitialized_) {
		DrawEffekseer2D();
	}

	if (isPause_) {
		UIManager::GetInstance().Draw(UIManager::UIState::Pause);
	}
}