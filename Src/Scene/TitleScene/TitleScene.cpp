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
#include <iomanip> // for setprecision

TitleScene::TitleScene(void)
{
    handle_ = -1;
    attackManager_ = new AttackManager();			// 攻撃管理の生成
    keyInputHandle_ = -1;
    keyInputHandleCmd_ = -1;
    registeredDisplayRemaining_ = 0;
    registeredDisplayMessage_.clear();
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
    registeredDisplayRemaining_ = 0;
    registeredDisplayMessage_.clear();
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
    auto it1 = std::find_if_not(t.begin(), t.end(), [](unsigned char c) { return std::isspace(static_cast<unsigned char>(c)); });
    auto it2 = std::find_if_not(t.rbegin(), t.rend(), [](unsigned char c) { return std::isspace(static_cast<unsigned char>(c)); }).base();
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
            // trim both (安全に unsigned char で ctype を扱う)
            auto trim = [](std::string& s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }).base(), s.end());
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
    // 生入力をトリム（先頭・末尾の空白のみ）
    std::string inputTrim = rawInput;
    inputTrim.erase(inputTrim.begin(), std::find_if(inputTrim.begin(), inputTrim.end(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }));
    inputTrim.erase(std::find_if(inputTrim.rbegin(), inputTrim.rend(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }).base(), inputTrim.end());
    if (inputTrim.empty()) {
        lastRegisteredCommand_ = "入力が空です";
        return;
    }

    // ASCII 小文字化 + trim（日本語は扱わない方針のため全角処理はしない）
    auto normalizeAsciiLowerOnly = [](const std::string& s)->std::string {
        std::string t = s;
        t.erase(t.begin(), std::find_if(t.begin(), t.end(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }));
        t.erase(std::find_if(t.rbegin(), t.rend(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }).base(), t.end());
        for (char& c : t) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc < 128) c = static_cast<char>(std::tolower(uc));
        }
        return t;
        };

    // CSV 辞書にあるか確認（入力そのまま、次に ASCII 小文字化したキーで検索）
    std::string keyLower = normalizeAsciiLowerOnly(inputTrim);
    auto it = commandMap_.find(inputTrim);
    if (it == commandMap_.end() && !keyLower.empty()) it = commandMap_.find(keyLower);

    // 見つからなければリセットして再入力を受け付ける（Update 側で入力窓はクリアされる）
    if (it == commandMap_.end()) {
        lastRegisteredCommand_ = "未登録のコマンドです。再入力してください。";

        // 入力窓の表示上の文字列（DxLib 側の内部バッファ）も消して最初から入力できるようにする
        if (keyInputHandleCmd_ != -1) {
            DeleteKeyInput(keyInputHandleCmd_);
            keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            // 再作成した main の入力ハンドルをアクティブにする（未登録時はすぐ再入力したい）
            SetActiveKeyInput(keyInputHandleCmd_);
        }
        // 外部に保持している表示用バッファもクリア
        cmdInputBuf_[0] = '\0';

        return;
    }

    // 見つかった場合、CSV の type（it->second）に従って処理する
    std::string rawType = it->second;
    // type をトリムして先頭 ':' 前までを使う（例: START:GAME）
    rawType.erase(rawType.begin(), std::find_if(rawType.begin(), rawType.end(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }));
    rawType.erase(std::find_if(rawType.rbegin(), rawType.rend(), [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }).base(), rawType.end());
    auto colon = rawType.find(':');
    std::string type = (colon != std::string::npos) ? rawType.substr(0, colon) : rawType;
    std::string typeLower = normalizeAsciiLowerOnly(type);

    // 判定集合（日本語語句は含めない方針：CSV の type で英語キーを使う想定）
    static const std::vector<std::string> REGISTER_KEYS = { "register", "ultimate", "registerultimate", "register_ultimate" };
    static const std::vector<std::string> START_KEYS = { "start", "play", "game", "begin", "startgame" };
    static const std::vector<std::string> LIST_KEYS = { "list", "commands", "help" };
    static const std::vector<std::string> EXIT_KEYS = { "exit", "quit", "close", "end" };

    auto isOneOf = [&](const std::string& v, const std::vector<std::string>& opts)->bool {
        for (const auto& o : opts) if (v == o) return true;
        return false;
        };

    // 必殺技登録
    if (isOneOf(typeLower, REGISTER_KEYS)) {
        // main の入力窓内テキストをクリアしておく（内部バッファも再作成して消す）
        if (keyInputHandleCmd_ != -1) {
            DeleteKeyInput(keyInputHandleCmd_);
            keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            // 登録モードへフォーカスを移すため main はアクティブにしない
            cmdInputBuf_[0] = '\0';
        }

        isRegisteringUltimate_ = true;
        keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
        SetActiveKeyInput(keyInputHandle_);
        inputBuf_[0] = '\0';
        lastRegisteredCommand_ = std::string("必殺技登録モード開始: ") + inputTrim;
        return;
    }
    // ゲーム開始
    if (isOneOf(typeLower, START_KEYS)) {
        lastRegisteredCommand_ = std::string("ゲーム開始コマンド: ") + inputTrim;
        SceneManager::GetInstance()->ChangeScene(SceneManager::SCENE_ID::GAME);
        return;
    }
    // コマンド一覧表示（CSV に登録された表示名一覧を使う）
    if (isOneOf(typeLower, LIST_KEYS)) {
        if (!commandNames_.empty()) {
            std::string out = "コマンド一覧: ";
            for (size_t i = 0; i < commandNames_.size(); ++i) {
                if (i) out += ", ";
                out += commandNames_[i];
            }
            lastRegisteredCommand_ = out;
        }
        else {
            lastRegisteredCommand_ = "コマンド一覧は空です";
        }
        return;
    }
    // 終了
    if (isOneOf(typeLower, EXIT_KEYS)) {
        lastRegisteredCommand_ = "終了コマンドによる終了";
        DxLib_End();
        exit(0);
        return;
    }

    // それ以外は type 情報を表示して終了
    lastRegisteredCommand_ = std::string("コマンド検出: ") + inputTrim + " -> " + rawType;
}
void TitleScene::Update(void)
{
    // 登録表示タイマーを更新（他のイベントに影響されず数秒表示）
    if (registeredDisplayRemaining_ > 0) {
        --registeredDisplayRemaining_;
        if (registeredDisplayRemaining_ == 0) {
            registeredDisplayMessage_.clear();
        }
    }

    // --- 必殺技コマンド登録モード ---
    if (!isRegisteringUltimate_) {
        // F1で登録モード開始（デバッグ用）
        if (CheckHitKey(KEY_INPUT_F1)) {
            // main の入力窓を先にクリアしておく
            if (keyInputHandleCmd_ != -1) {
                DeleteKeyInput(keyInputHandleCmd_);
                keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
                cmdInputBuf_[0] = '\0';
            }

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
            // 登録完了後は main の入力窓に戻してバッファをクリアしておく
            if (keyInputHandleCmd_ == -1) {
                keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            }
            SetActiveKeyInput(keyInputHandleCmd_); // 入力窓へ戻す
            keyInputHandle_ = -1;
            isRegisteringUltimate_ = false;
            if (attackManager_ && inputBuf_[0] != '\0') {
                std::string commandStr(inputBuf_);
                std::string commandId = attackManager_->RegisterUltimateCommand(commandStr, 5);
                attackManager_->ReloadCommands();

                // 登録成功時：生成パラメータを取得して数秒表示
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
                    lastRegisteredCommand_ = registeredDisplayMessage_; // 状態欄も更新
                }
                else {
                    lastRegisteredCommand_ = "登録に失敗しました";
                }
            }
            else {
                lastRegisteredCommand_ = "登録に失敗しました";
            }
            inputBuf_[0] = '\0';
            // main 側表示バッファもクリア
            cmdInputBuf_[0] = '\0';
            // アクティブに戻した main ハンドルを再セット（安全のため）
            SetActiveKeyInput(keyInputHandleCmd_);
        }
        // Escでキャンセル
        if (CheckHitKey(KEY_INPUT_ESCAPE)) {
            DeleteKeyInput(keyInputHandle_);
            if (keyInputHandleCmd_ == -1) {
                keyInputHandleCmd_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
            }
            SetActiveKeyInput(keyInputHandleCmd_); // 入力窓へ戻す
            keyInputHandle_ = -1;
            isRegisteringUltimate_ = false;
            inputBuf_[0] = '\0';
            cmdInputBuf_[0] = '\0';
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

            // 重要: ProcessTitleCommand が必殺技登録へ遷移させた場合は
            // main ハンドルを再アクティブ化しない（F1 経由と同様の挙動）
            if (!isRegisteringUltimate_) {
                // 入力窓は継続して使用するのでバッファのみクリア
                cmdInputBuf_[0] = '\0';
                // 再アクティブ化（安全のため）
                SetActiveKeyInput(keyInputHandleCmd_);
            }
            else {
                // 登録モードへ移行したため main のバッファだけクリアして処理を継続
                cmdInputBuf_[0] = '\0';
            }
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

    int screenW = 0;
    int screenH = 0;
    // DxLib の画面サイズ取得関数を使用
    GetDrawScreenSize(&screenW, &screenH);
    // 入力窓の想定幅（DxLib の入力描画は幅固定ではないが、表示位置調整用に使用）
    const int inputBoxW = 800;
    const int inputBoxH = 32;
    int candidateX = screenW / 2 - inputBoxW / 2;
    int inputX = (candidateX > 0) ? candidateX : 0;
    int inputY = screenH / 2 - inputBoxH / 2;

    // 背景画像（必要なら表示）
    // DrawGraph(0, 0, handle_, true);

    // 入力窓表示（中央）
    DrawString(inputX, inputY - 56, "入力窓: 文言を入力してEnter（CSV参照）", GetColor(255, 255, 255));
    if (keyInputHandleCmd_ != -1) {
        DrawKeyInputString(inputX, inputY, keyInputHandleCmd_);
        DrawFormatString(inputX, inputY + 40, GetColor(0, 255, 0), "入力: %s", cmdInputBuf_);
    }

    if (isRegisteringUltimate_) {
        DrawString(inputX, inputY - 56, "必殺技登録モード: 名前を入力してEnter（Escでキャンセル）", GetColor(255, 255, 255));
        if (keyInputHandle_ != -1) {
            DrawKeyInputString(inputX, inputY, keyInputHandle_);
        }
        DrawFormatString(inputX, inputY + 40, GetColor(0, 255, 0), "登録用入力: %s", inputBuf_);
    }
    else {
        DrawString(inputX, inputY + 80, "F1で必殺技登録モード  SPACEでゲーム開始", GetColor(255, 255, 255));
        // 登録済みコマンド名を表示（状態）
        const std::string& statusMsg = (!registeredDisplayMessage_.empty()) ? registeredDisplayMessage_ : lastRegisteredCommand_;
        if (!statusMsg.empty()) {
            DrawFormatString(inputX, inputY + 112, GetColor(255, 255, 0), "状態: %s", statusMsg.c_str());
        }
    }

    // 説明テキストは左寄せでそのままに（必要なら中央化も可能）
    DrawString(50, 800, "このバトルタイピングは、コマンドをtypingして、回避や移動、攻撃、必殺、などを繰り出しボスを倒すのが目的です\nコマンドは、みぎ、ひだり、うえ、した、みぎにいどう、などいろいろあります\nコマンドはゲームシーンでTAB押下で確認することができます\n攻撃コマンドもこうげき、はっしゃなど複数コマンドがあります\n必殺技は各々が命名して生成することができます(6文字以上)。技によってステータスが変わります。強い必殺技やかっこいい必殺技を生成しましょう", GetColor(255, 255, 255));

    if (attackManager_) {
        int candidateRightX = screenW - 600;
        int x = (candidateRightX > 0) ? candidateRightX : 0; // 右側へ寄せる（std::max を使わない）
        int y = 100;
        int lineHeight = 32;
        int maxDisplay = 20;
        int listSize = static_cast<int>(attackManager_->registeredCommands_.size());
        DrawString(x, 80, "必殺技一覧", GetColor(255, 255, 0));
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