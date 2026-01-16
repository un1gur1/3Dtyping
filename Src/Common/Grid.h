#pragma once
#include <DxLib.h>
class Camera;
class Grid {
public:
    enum class GridState {
        Normal,
        Warning,
        Attack
    };
    static constexpr int MAGIC_CIRCLE_WIDTH = 1500;
    static constexpr int MAGIC_CIRCLE_HEIGHT = 1500;

    // メンバ変数
    VECTOR pos_;         // 中心座標
    float angle_;        // 回転角度
    float scale_;        // 基本スケール
    int imageHandle_;    // 魔法陣画像ハンドル

    GridState state_;    // 現在の状態

    void Init(float x, float y);
    void Update();
    void Draw();

    static VECTOR GetWorldPosFromIndex(int index, bool isPlayerSide);

 

private:
	Camera* camera_;

};
