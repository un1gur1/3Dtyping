#include "TitleScene.h"

#include <DxLib.h>

#include "../../Input/InputManager.h"
#include "../SceneManager.h"
#include "../../Object/Attack/AttackManager.h"
#include "../../Common/UiManager.h"

TitleScene::TitleScene(void)
{
	handle_ = -1;
    attackManager_ = new AttackManager();			// 攻撃管理の生成

}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
}

void TitleScene::Load(void)
{
	handle_ = LoadGraph("Data/Image/Title2.png");
}

void TitleScene::LoadEnd(void)
{
	Init();
}

void TitleScene::Update(void)
{
    // --- 必殺技コマンド登録モード ---
    if (!isRegisteringUltimate_) {
        // F1で登録モード開始
        if (CheckHitKey(KEY_INPUT_F1)) {
            isRegisteringUltimate_ = true;
            keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            SetActiveKeyInput(keyInputHandle_);
            inputBuf_[0] = '\0';
        }
    }
    else {
        // 入力受付中
        GetKeyInputString(inputBuf_, keyInputHandle_);
        // Enterで登録
        if (CheckKeyInput(keyInputHandle_) == 1) {
            DeleteKeyInput(keyInputHandle_);
            SetActiveKeyInput(-1);
            isRegisteringUltimate_ = false;
            if (attackManager_ && inputBuf_[0] != '\0') {
                std::string commandStr(inputBuf_);
                std::string commandId = attackManager_->RegisterUltimateCommand(commandStr, 5);
                attackManager_->ReloadCommands();

                // コマンドIDでデータを取得
                auto it = attackManager_->ultimateCommandDataMap_.find(commandId);
                /*if (it != attackManager_->ultimateCommandDataMap_.end()) {
                    const auto& data = it->second;
                    printfDx("登録: %s, ID: %s, ダメージ: %d, 速度: %.1f\n",
                        commandStr.c_str(), commandId.c_str(), data.damage, data.speed);
                }
                else {
                    printfDx("登録: %s, ID: %s\n", commandStr.c_str(), commandId.c_str());
                }*/
            }
            inputBuf_[0] = '\0';
        }
        // Escでキャンセル
        if (CheckHitKey(KEY_INPUT_ESCAPE)) {
            DeleteKeyInput(keyInputHandle_);
            SetActiveKeyInput(-1);
            isRegisteringUltimate_ = false;
            inputBuf_[0] = '\0';
        }
        return;
    }
    //    // ポーズ切り替え
    //    if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_TAB)) {
    //        if (pauseState_ == PauseMenuState::None) {
    //            pauseState_ = PauseMenuState::Pause;
    //            pauseCursor_ = 0;
    //            isPause_ = true;
    //        }
    //        else {
    //            pauseState_ = PauseMenuState::None;
    //            isPause_ = false;
    //        }
    //    }
    //    // ポーズ中の操作
    //    if (isPause_) {
    //        if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_UP)) {
    //            pauseCursor_ = (pauseCursor_ + 2) % 3;
    //        }
    //        if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_DOWN)) {
    //            pauseCursor_ = (pauseCursor_ + 1) % 3;
    //        }
    //        if (InputManager::GetInstance()->IsTrgUp(KEY_INPUT_RETURN)) {
    //            if (pauseCursor_ == 1) {
    //                // タイトルに戻る（タイトルなので何もしない or シーンリセット）
    //            }
    //            else if (pauseCursor_ == 2) {
    //                DxLib_End();
    //                exit(0);
    //            }
    //        }
    //        UIManager::GetInstance().SetPauseCursor(pauseCursor_);
    //        UIManager::GetInstance().Update(UIManager::UIState::Pause);
    //        return;
    //    }
    //}
    // 必殺技一覧スクロール
    if (!isRegisteringUltimate_ && attackManager_) {
        int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
        if (CheckHitKey(KEY_INPUT_UP)) {
            if (scrollOffset_ > 0) scrollOffset_--;
        }
        if (CheckHitKey(KEY_INPUT_DOWN)) {
            if (scrollOffset_ < listSize - 1) scrollOffset_++;
        }
    }

    // スペースで次のシーンへ
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
    if (isRegisteringUltimate_) {
        DrawString(50, 900, "必殺技コマンドを入力してEnterで登録（Escでキャンセル）", GetColor(255, 255, 255));
        DrawKeyInputString(50, 730, keyInputHandle_);
        // 入力中のコマンドを表示
        DrawFormatString(50, 950, GetColor(0, 255, 0), "現在入力中: %s", inputBuf_);
    }
    else {
        DrawString(50, 980, "F1で必殺技登録モード\n\nSPACEでゲーム開始", GetColor(255, 255, 255));
        // 登録済みコマンド名を表示
        if (!lastRegisteredCommand_.empty()) {
            DrawFormatString(50, 950, GetColor(255, 255, 0), "登録したコマンド: %s", lastRegisteredCommand_.c_str());
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
			DrawString(1500,80,"必殺技一覧", GetColor(255, 255, 0));
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
