C++ Src\Scene\TitleScene\TitleScene.cpp
#include "TitleScene.h"

#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"
#include "../../Object/Attack/AttackManager.h"
#include "../../Common/UiManager.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

TitleScene::TitleScene(void)
{
    handle_ = -1;
    attackManager_ = new AttackManager();			// 攻撃管理の生成
    keyInputHandle_ = -1;
    keyInputHandleCmd_ = -1;
}

TitleScene::~TitleScene(void)
{
    // 入力ハンドルのクリーンアップ
    if (keyInputHandle_ != -1) {
        DeleteKeyInput(keyInputHandle_);
        keyInputHandle_ = -1;
    }
    if (keyInputHandleCmd_ != -1) {
        DeleteKeyInput(keyInputHandleCmd_);
        keyInputHandleCmd_ = -1;
    }
    delete attackManager_;
}

void TitleScene::Init(void)
{
    // 常時表示する入力窓を作成してアクティブにする（タイトルでは入力窓がメイン）
    keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
    SetActiveKeyInput(keyInputHandleCmd_);
    cmdInputBuf_[0] = '\0';
}

void TitleScene::Load(void)
{
    handle_ = LoadGraph("Data/Image/Title2.png");
    // CSV からコマンド名とタイプを読み込む（コマンドCSV のパスはプロジェクトに合わせて変更）
    LoadCommandsFromCSV("Data/CSV/Word.csv");
}

void TitleScene::LoadEnd(void)
{
    Init();
}

static std::string ToLowerTrim(const std::string& s) {
    std::string t = s;
    // trim
    auto it1 = std::find_if_not(t.begin(), t.end(), [](unsigned char c) { return std::isspace(c); });
    auto it2 = std::find_if_not(t.rbegin(), t.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    if (it1 >= it2) return std::string();
    t = std::string(it1, it2);
    // lowercase (ASCII)
    for (char& c : t) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return t;
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
            // trim both
            auto trim = [](std::string& s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){ return !std::isspace(ch); }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){ return !std::isspace(ch); }).base(), s.end());
            };
            trim(name); trim(type);
            if (name.empty()) continue;
            commandMap_[name] = type;
            commandNames_.push_back(name);
            // 追加で小文字キーも登録（ASCIIのみ想定）
            std::string lower = ToLowerTrim(name);
            if (!lower.empty() && lower != name) {
                // 原文キーも残しつつ lookup 用に小文字を登録（衝突は上書きされるが許容）
                commandMap_[lower] = type;
            }
        }
    }
    lastRegisteredCommand_ = "コマンドCSV読み込み完了";
}

void TitleScene::ProcessTitleCommand(const std::string& rawInput)
{
    // 生入力トリム
    std::string inputTrim = rawInput;
    inputTrim.erase(inputTrim.begin(), std::find_if(inputTrim.begin(), inputTrim.end(), [](int ch){ return !std::isspace(ch); }));
    inputTrim.erase(std::find_if(inputTrim.rbegin(), inputTrim.rend(), [](int ch){ return !std::isspace(ch); }).base(), inputTrim.end());
    if (inputTrim.empty()) {
        lastRegisteredCommand_ = "入力が空です";
        return;
    }

    // まず CSV 辞書に問い合わせる（原文キーと小文字キーを順に試す）
    std::string keyLower = ToLowerTrim(inputTrim);
    auto it = commandMap_.find(inputTrim);
    if (it == commandMap_.end() && !keyLower.empty()) it = commandMap_.find(keyLower);

    if (it != commandMap_.end()) {
        const std::string type = it->second;
        // CSV の type によって振る舞いを決定
        // 小文字比較でいくつかの代表的なタイプを扱う
        std::string typeLower = ToLowerTrim(type);
        if (typeLower == "register" || typeLower == "ultimate" || typeLower == "registerultimate") {
            // 必殺技登録モード
            isRegisteringUltimate_ = true;
            keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            SetActiveKeyInput(keyInputHandle_);
            inputBuf_[0] = '\0';
            lastRegisteredCommand_ = std::string("必殺技登録モード開始: ") + inputTrim;
            return;
        }
        if (typeLower == "start" || typeLower == "play" || typeLower == "game") {
            lastRegisteredCommand_ = std::string("ゲーム開始コマンド: ") + inputTrim;
            SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
            return;
        }
        if (typeLower == "list" || typeLower == "commands" || typeLower == "help") {
            // コマンド一覧表示
            if (!commandNames_.empty()) {
                std::string out = "コマンド一覧: ";
                for (size_t i = 0; i < commandNames_.size(); ++i) {
                    if (i) out += ", ";
                    out += commandNames_[i];
                }
                lastRegisteredCommand_ = out;
            } else {
                lastRegisteredCommand_ = "コマンド一覧は空です";
            }
            return;
        }
        if (typeLower == "exit" || typeLower == "quit" || typeLower == "close") {
            lastRegisteredCommand_ = "終了コマンドによる終了";
            DxLib_End();
            exit(0);
            return;
        }

        // 上記に該当しないタイプは情報表示のみ
        lastRegisteredCommand_ = std::string("コマンド: ") + inputTrim + " (type:" + type + ")";
        return;
    }

    // CSV にない場合は従来のフレーズ判定（日本語／英語の簡易同義語）
    std::string lower = ToLowerTrim(inputTrim);
    if (lower == "register" || lower == "registerultimate" || lower == "登録" || lower == "必殺登録") {
        isRegisteringUltimate_ = true;
        keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
        SetActiveKeyInput(keyInputHandle_);
        inputBuf_[0] = '\0';
        lastRegisteredCommand_ = "必殺技登録モードに入りました";
        return;
    }
    if (lower == "play" || lower == "start" || lower == "game" || lower == "はじめる" || lower == "ゲーム") {
        lastRegisteredCommand_ = "ゲーム開始";
        SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
        return;
    }
    if (lower == "commands" || lower == "list" || lower == "help" || lower == "一覧" || lower == "コマンド") {
        if (attackManager_ && !attackManager_->registeredCommands_.empty()) {
            std::string out = "必殺技一覧: ";
            for (size_t i = 0; i < attackManager_->registeredCommands_.size(); ++i) {
                if (i) out += ", ";
                out += attackManager_->registeredCommands_[i].first;
            }
            lastRegisteredCommand_ = out;
        } else {
            lastRegisteredCommand_ = "登録済みコマンドはありません";
        }
        return;
    }
    if (lower == "exit" || lower == "quit" || lower == "終了") {
        lastRegisteredCommand_ = "終了します";
        DxLib_End();
        exit(0);
        return;
    }

    // 最終的に未認識
    lastRegisteredCommand_ = std::string("不明なコマンド: ") + inputTrim;
}

void TitleScene::Update(void)
{
    // --- 必殺技コマンド登録モード ---
    if (!isRegisteringUltimate_) {
        // F1で登録モード開始（デバッグ用）
        if (CheckHitKey(KEY_INPUT_F1)) {
            isRegisteringUltimate_ = true;
            keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            SetActiveKeyInput(keyInputHandle_);
            inputBuf_[0] = '\0';
        }
    }
    else {
        // 登録入力受付中（Enter で登録）
        GetKeyInputString(inputBuf_, keyInputHandle_);
        if (CheckKeyInput(keyInputHandle_) == 1) {
            DeleteKeyInput(keyInputHandle_);
            SetActiveKeyInput(keyInputHandleCmd_); // 入力窓へ戻す
            keyInputHandle_ = -1;
            isRegisteringUltimate_ = false;
            if (attackManager_ && inputBuf_[0] != '\0') {
                std::string commandStr(inputBuf_);
                std::string commandId = attackManager_->RegisterUltimateCommand(commandStr, 5);
                attackManager_->ReloadCommands();
                lastRegisteredCommand_ = std::string("登録しました: ") + commandStr;
            }
            else {
                lastRegisteredCommand_ = "登録に失敗しました";
            }
            inputBuf_[0] = '\0';
        }
        // Escでキャンセル
        if (CheckHitKey(KEY_INPUT_ESCAPE)) {
            DeleteKeyInput(keyInputHandle_);
            SetActiveKeyInput(keyInputHandleCmd_); // 入力窓へ戻す
            keyInputHandle_ = -1;
            isRegisteringUltimate_ = false;
            inputBuf_[0] = '\0';
        }
        return;
    }

    // --- メイン入力窓（常時） ---
    if (keyInputHandleCmd_ != -1) {
        GetKeyInputString(cmdInputBuf_, keyInputHandleCmd_);
        // Enter で確定
        if (CheckKeyInput(keyInputHandleCmd_) == 1) {
            std::string raw(cmdInputBuf_);
            ProcessTitleCommand(raw);
            // 入力窓は継続して使用するのでバッファのみクリア
            cmdInputBuf_[0] = '\0';
            // 再アクティブ化（安全のため）
            SetActiveKeyInput(keyInputHandleCmd_);
        }
    }

    // 必殺技一覧スクロール (表示のみ)
    if (!isRegisteringUltimate_ && attackManager_) {
        int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
        if (CheckHitKey(KEY_INPUT_UP)) {
            if (scrollOffset_ > 0) scrollOffset_--;
        }
        if (CheckHitKey(KEY_INPUT_DOWN)) {
            if (scrollOffset_ < listSize - 1) scrollOffset_++;
        }
    }

    // スペースで次のシーンへ（従来挙動）
    if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_SPACE))
    {
        SceneManager::GetInstance()->ChangeScene(
            SceneManager::SCENE_ID::GAME);
    }
}

void TitleScene::Draw(void)
{
    SetBackgroundColor(0, 0, 0);

    DrawGraph(0, 0, handle_, true);

    // 入力窓表示（常時）
    DrawString(50, 900, "入力窓: 文言を入力してEnter（CSV参照）", GetColor(255, 255, 255));
    if (keyInputHandleCmd_ != -1) {
        DrawKeyInputString(50, 730, keyInputHandleCmd_);
        DrawFormatString(50, 950, GetColor(0, 255, 0), "入力: %s", cmdInputBuf_);
    }

    if (isRegisteringUltimate_) {
        DrawString(50, 840, "必殺技登録モード: 名前を入力してEnter（Escでキャンセル）", GetColor(255, 255, 255));
        DrawKeyInputString(50, 730, keyInputHandle_);
        DrawFormatString(50, 980, GetColor(0, 255, 0), "登録用入力: %s", inputBuf_);
    }
    else {
        DrawString(50, 980, "F1で必殺技登録モード\nSPACEでゲーム開始", GetColor(255, 255, 255));
        // 登録済みコマンド名を表示（状態）
        if (!lastRegisteredCommand_.empty()) {
            DrawFormatString(50, 950, GetColor(255, 255, 0), "状態: %s", lastRegisteredCommand_.c_str());
        }
    }

    DrawGraph(0, 0, handle_, true);
    DrawString(50, 800, "このバトルタイピングは、コマンドをtypingして、回避や移動、攻撃、必殺、などを繰り出しボスを倒すのが目的です\nコマンドは、みぎ、ひだり、うえ、した、みぎにいどう、などいろいろあります\nコマンドはゲームシーンでTAB押下で確認することができます\n攻撃コマンドもこうげき、はっしゃなど複数コマンドがあります\n必殺技は各々が命名して生成することができます(6文字以上)。技によってステータスが変わります。強い必殺技やかっこいい必殺技を生成しましょう", GetColor(255, 255, 255));

    if (attackManager_) {
        int x = 1500;
        int y = 100;
        int lineHeight = 32;
        int maxDisplay = 20;
        int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
        for (int i = 0; i < maxDisplay; ++i) {
            int idx = i + scrollOffset_;
            if (idx >= listSize) break;
            const auto& pair = attackManager_->registeredCommands_[idx];
            const std::string& commandId = pair.second; // コマンドID
            int damage = 0;
            // ダメージ情報を取得
            auto it = attackManager_->ultimateCommandDataMap_.find(commandId);
            if (it != attackManager_->ultimateCommandDataMap_.end()) {
                damage = it->second.damage;
            }
            DrawString(1500, 80, "必殺技一覧", GetColor(255, 255, 0));
            DrawFormatString(x, y + i * lineHeight, GetColor(255, 255, 0),
                "%2d: %s [DMG:%d]", idx + 1, pair.first.c_str(), damage);
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
    DeleteGraph(handle_);
    // ハンドルはデストラクタでも消しているが念のためここでも
    if (keyInputHandle_ != -1) {
        DeleteKeyInput(keyInputHandle_);
        keyInputHandle_ = -1;
    }
    if (keyInputHandleCmd_ != -1) {
        DeleteKeyInput(keyInputHandleCmd_);
        keyInputHandleCmd_ = -1;
    }
}