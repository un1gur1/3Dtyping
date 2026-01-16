#pragma once
#include <string>
#include <vector>
#include"../Application.h"
#include "Grid.h"
class Camera;
class UIManager {
public:

    // UIの状態
    enum class UIState {
        Normal,     // 通常戦闘
        Decision,   // 「迎え撃つ」選択
        Clash,      // 相殺連打
        Stun,       // チャンスタイム（怯み）
        Pause       // ポーズ画面
    };

    // シングルトンインスタンス取得
    static UIManager& GetInstance() {
        static UIManager instance;
        return instance;
    }

    // コピー・代入禁止
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    void InitGrids();
    // 毎フレーム呼び出し
    void Update(UIState currentState);
    void Draw(UIState currentState);

    // 外部からデータをセット
    void SetPlayerStatus(int hp, int maxHp);
    void SetEnemyStatus(int hp, int maxHp, const std::string& name);
    void SetTypingData(const std::string& target, const std::string& input);
    void SetClashData(int currentPower, float remainingTime);
    void SetPauseCursor(int cursor);

    void SetGridState(int index, Grid::GridState state, bool isPlayerSide);

    // --- UI定数 ---
    static const int SCREEN_WIDTH = Application::SCREEN_SIZE_X;
    static const int SCREEN_HEIGHT = Application::SCREEN_SIZE_Y;
    static constexpr int BOX_WIDTH = 1980;
    static constexpr int BOX_HEIGHT = 300;
    static constexpr int BOX_X = (SCREEN_WIDTH - BOX_WIDTH) / 2;
    static constexpr int BOX_Y = 40;
    static constexpr int FONT_SIZE_LARGE = 64;
    static constexpr int INPUT_TEXT_X = BOX_X + 60;
    static constexpr int INPUT_TEXT_Y = BOX_Y + 40;
    static constexpr int HIRA_TEXT_Y = BOX_Y + 110;
    static constexpr int PREV_INPUT_Y = BOX_Y + 170;
    static constexpr int ANGLE_TEXT_X = 0;
    static constexpr int ANGLE_TEXT_Y = 50;
    static constexpr int BULLET_MSG_X = 10;
    static constexpr int BULLET_MSG_Y = 160;
    static constexpr int HP_TEXT_X = 800;
    static constexpr int HP_TEXT_Y = 10;
    static constexpr int REGISTER_BOX_LEFT = 400;
    static constexpr int REGISTER_BOX_TOP = 300;
    static constexpr int REGISTER_BOX_RIGHT = 1200;
    static constexpr int REGISTER_BOX_BOTTOM = 400;
    static constexpr int REGISTER_TEXT1_X = 420;
    static constexpr int REGISTER_TEXT1_Y = 320;
    static constexpr int REGISTER_TEXT2_X = 420;
    static constexpr int REGISTER_TEXT2_Y = 360;
    static constexpr unsigned int COLOR_MAGENTA = 0xff00ff;
    static constexpr unsigned int COLOR_GREEN = 0x00ff00;
    static constexpr unsigned int COLOR_WHITE = 0xffffff;
    static constexpr unsigned int COLOR_BLACK = 0x000000;

    // タイピング表示用メンバ変数
    std::string typingInput_;        // 現在の入力（ローマ字）
    std::string typingConverted_;    // 変換後（ひらがな等）
    std::string typingPrevInput_;    // 直前の入力（ローマ字）
    std::string typingPrevConverted_;// 直前の変換後

    // 外部から表示用文字列をセット
    void SetTypingStrings(
        const std::string& input,
        const std::string& converted,
        const std::string& prevInput,
        const std::string& prevConverted
    );

    // HPバー描画
    void DrawHpBar(int x, int y, int width, int height, int hp, int maxHp, unsigned int color, const char* label);

    std::vector<std::string> normalCommandList_;
    std::vector<std::string> ultimateCommandList_;

    void SetNormalCommandList(const std::vector<std::string>& list);
    void SetUltimateCommandList(const std::vector<std::string>& list);
    // グリッド状態（5マス分、必要に応じてサイズ調整）
    std::vector<Grid> playerGrids_ = std::vector<Grid>(5);
    std::vector<Grid> enemyGrids_ = std::vector<Grid>(5);

    std::vector<Grid> grids;


private:
    bool isGridInitialized = false;

    UIManager() = default;
    ~UIManager() = default;

    void DrawCommonHUD();
    void DrawTypingArea();
    void DrawClashEvent();
    void DrawPauseMenu();

    // --- 保持データ ---
    UIState state_ = UIState::Normal;
    int playerHp_ = 0;
    int playerMaxHp_ = 0;
    int enemyHp_ = 0;
    int enemyMaxHp_ = 0;
    std::string enemyName_;
    std::string targetWord_;
    std::string inputWord_;
    int clashPower_ = 0;
    float timer_ = 0.0f;
    int pauseCursor_ = 0;

	Camera* camera = nullptr;

    int debugGridIndex_ = 0; // 動かすグリッドのインデックス
    float debugOffsetX_ = 0.0f;
    float debugOffsetY_ = 0.0f;
};
