#include "TitleScene.h"

#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"
#include "../../Object/Attack/AttackManager.h"
#include "../../Common/UiManager.h"

#include <fstream>
#include <sstream>
#include <algorithm>

TitleScene::TitleScene(void)
{
    handle_ = -1;
    attackManager_ = new AttackManager();			// 攻撃管理の生成
}

TitleScene::~TitleScene(void)
{
    // 入力ハンドルが残ってたら掃除
    if (keyInputHandle_ != -1) {
        DeleteKeyInput(keyInputHandle_);
        SetActiveKeyInput(-1);
    }
    delete attackManager_;
}

void TitleScene::Init(void)
{
    // タイトル到着時に常時コマンド入力バーを有効にする
    if (keyInputHandle_ == -1) {
        keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
        SetActiveKeyInput(keyInputHandle_);
        inputBuf_[0] = '\0';
        romanjiConverter_.clear();
    }
}

void TitleScene::Load(void)
{
    handle_ = LoadGraph("Data/Image/Title2.png");
    // 床コマンドを読み込む（Player と同じ CSV フォーマットを使う）
    LoadFloorCommands("Data/CSV/Word.csv");
}

void TitleScene::LoadEnd(void)
{
    Init();
}

void TitleScene::LoadFloorCommands(const std::string& path)
{
    floorCommandMap_.clear();
    floorCommands_.clear();

    std::ifstream file(path);
    if (!file.is_open()) {
        lastRegisteredCommand_ = "床コマンドCSV読み込み失敗: " + path;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word, typeStr;
        if (std::getline(iss, word, ',') &&
            std::getline(iss, typeStr, ',')) {
            // trim whitespace
            auto trim = [](std::string& s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
                };
            trim(word); trim(typeStr);
            if (!word.empty()) {
                floorCommandMap_[word] = typeStr;
                floorCommands_.push_back(word);
            }
        }
    }
    lastRegisteredCommand_ = "床コマンド読み込み完了";
}

void TitleScene::ProcessFloorCommand(const std::string& rawInput)
{
    // 生入力をそのまま比較（必要なら小文字化や全角→半角等の正規化を追加）
    std::string input = rawInput;
    // trim
    input.erase(input.begin(), std::find_if(input.begin(), input.end(), [](int ch) { return !std::isspace(ch); }));
    input.erase(std::find_if(input.rbegin(), input.rend(), [](int ch) { return !std::isspace(ch); }).base(), input.end());
    if (input.empty()) {
        lastRegisteredCommand_ = "入力が空です";
        return;
    }

    // 1) 床コマンドに存在するかチェック
    auto it = floorCommandMap_.find(input);
    bool foundOnFloor = (it != floorCommandMap_.end());

    // 2) 登録済み必殺技に存在するかチェック
    bool foundUltimate = false;
    std::string foundUltimateId;
    if (attackManager_) {
        for (const auto& p : attackManager_->registeredCommands_) {
            if (p.first == input) {
                foundUltimate = true;
                foundUltimateId = p.second;
                break;
            }
        }
    }

    if (!foundOnFloor && !foundUltimate) {
        lastRegisteredCommand_ = "床にそのコマンドはありません: " + input;
        return;
    }

    // 優先: 床にある通常コマンド (typeStr) によるアクション
    if (foundOnFloor) {
        const std::string type = it->second;
        // 簡易ルール:
        // - 移動/攻撃系のタイプならプレイ開始に移行
        // - type が特定のキーワードなら専用処理
        if (type.rfind("MOVE", 0) == 0 || type == "ATTACK" || type == "SHOOT" || type == "MOVE_RANDOM" || type == "DODGE") {
            lastRegisteredCommand_ = "床コマンド確認: " + input + " → ゲーム開始";
            SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
            return;
        }
        // 登録トリガーを床コマンドで表現したい場合 (例: type == "REGISTER")
        if (type == "REGISTER" || type == "ULTIMATE") {
            // 必殺技登録モードへ（ここではタイトル上で直接登録したければ専用処理を記述）
            lastRegisteredCommand_ = "必殺技登録トリガー検出: " + input;
            // 必要ならここで登録モードへ遷移する処理を追加
            return;
        }

        // 既定アクションがない場合は情報表示
        lastRegisteredCommand_ = "床コマンド検出: " + input + " (type:" + type + ")";
        return;
    }

    // 最後に必殺技名でヒットした場合の処理
    if (foundUltimate) {
        // 必殺技名を選択した扱いにする（表示のみ）
        lastRegisteredCommand_ = "必殺技選択: " + input + " (ID:" + foundUltimateId + ")";
        // ここに「選択してゲームで使う」等の処理を追加可能
        return;
    }
}

void TitleScene::Update(void)
{
    // 常時コマンド入力バーで操作（Init で MakeKeyInput されている前提）
    if (keyInputHandle_ == -1) {
        keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
        SetActiveKeyInput(keyInputHandle_);
        inputBuf_[0] = '\0';
    }

    // 入力を取得して Enter で確定（確定後も入力バーは維持）
    GetKeyInputString(inputBuf_, keyInputHandle_);
    if (CheckKeyInput(keyInputHandle_) == 1) {
        std::string raw(inputBuf_);
        ProcessFloorCommand(raw);

        // 入力ハンドルをリセットしてバッファクリア（内部状態を確実に空にするため）
        DeleteKeyInput(keyInputHandle_);
        SetActiveKeyInput(-1);
        keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
        SetActiveKeyInput(keyInputHandle_);
        inputBuf_[0] = '\0';
        romanjiConverter_.clear();
    }
    // Esc で入力をキャンセル（バッファクリア）
    if (CheckHitKey(KEY_INPUT_ESCAPE)) {
        DeleteKeyInput(keyInputHandle_);
        SetActiveKeyInput(-1);
        keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
        SetActiveKeyInput(keyInputHandle_);
        inputBuf_[0] = '\0';
        romanjiConverter_.clear();
        lastRegisteredCommand_ = "入力キャンセル";
    }

    // 必殺技一覧スクロール（既存）
    if (attackManager_) {
        int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
        if (CheckHitKey(KEY_INPUT_UP)) {
            if (scrollOffset_ > 0) scrollOffset_--;
        }
        if (CheckHitKey(KEY_INPUT_DOWN)) {
            if (scrollOffset_ < listSize - 1) scrollOffset_++;
        }
    }

    // 従来のスペースでゲーム開始は残す（必要なければ削除可）
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

    // 常時表示するコマンド入力バー
    DrawString(50, 880, "コマンド入力バー: コマンドをタイプしてEnterで決定（Escでキャンセル）", GetColor(255, 255, 255));
    DrawKeyInputString(50, 730, keyInputHandle_);

    // ローマ字→ひらがな表示（Player と同様に毎フレーム構築）
    romanjiConverter_.clear();
    for (int i = 0; inputBuf_[i] != '\0'; ++i) {
        romanjiConverter_.addInput(inputBuf_[i]);
    }
    std::string hiraText = romanjiConverter_.getOutput();
    DrawFormatString(50, 950, GetColor(0, 255, 0), "入力(原文): %s", inputBuf_);
    DrawFormatString(50, 980, GetColor(0, 200, 200), "ひらがな表示: %s", hiraText.c_str());

    // 状態表示
    if (!lastRegisteredCommand_.empty()) {
        DrawFormatString(50, 920, GetColor(255, 255, 0), "状態: %s", lastRegisteredCommand_.c_str());
    }

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
}