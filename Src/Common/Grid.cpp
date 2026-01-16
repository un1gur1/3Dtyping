#include "Grid.h"
#include "../Camera/Camera.h"
#include"../Application.h"
#include<DxLib.h>
// 初期化
void Grid::Init(float x, float y) {
    pos_ = { x, y, 0.0f };
    state_ = GridState::Normal;
    angle_ = 0.0f;
    scale_ = 0.26f;
    imageHandle_ = LoadGraph("Data/Model/Stage/magic_circle.png");

}

// 毎フレーム更新
void Grid::Update() {
    //// 毎フレームY軸回転（速度は調整可、例: 1度ずつ回転）
    //angle_ += 1.0f * DX_PI_F / 180.0f;

    //// 正規化（0～2π）
    //angle_ = fmodf(angle_, DX_TWO_PI_F);
    //if (angle_ < 0.0f) angle_ += DX_TWO_PI_F;
}

void Grid::Draw() {
    if (imageHandle_ == -1) return;

    // 1. 頂点データを作る（モデル空間：中心を 0,0,0 とした板）
    VERTEX3D v[6];
    float size = 200.0f; // 1マスの半分

    // 共通設定
    for (int i = 0; i < 6; i++) {
        v[i].dif = GetColorU8(255, 255, 255, 255);
        v[i].spc = GetColorU8(0, 0, 0, 0);
        v[i].u = v[i].v = 0.0f; 
    }

    // 基本となる4点の座標（XZ平面に水平）
    VECTOR p1 = VGet(-size, 0.1f, size); // 左上
    VECTOR p2 = VGet(size, 0.1f, size); // 右上
    VECTOR p3 = VGet(-size, 0.1f, -size); // 左下
    VECTOR p4 = VGet(size, 0.1f, -size); // 右下

    // 2. 「回転」と「移動」の行列を作る
    MATRIX rotMat = MGetRotY(angle_);           // Y軸まわりに自転
    MATRIX transMat = MGetIdent();
    transMat = MGetTranslate(VGet(pos_.x, 0.0f, pos_.y)); // ワールド座標(pos_)へ移動

    // 3. 行列を合成（回転してから移動）
    MATRIX worldMat = MMult(rotMat, transMat);

    // 4. 全ての頂点に行列を適用してワールド座標に変換
    v[0].pos = VTransform(p1, worldMat); v[0].u = 0.0f; v[0].v = 0.0f;
    v[1].pos = VTransform(p2, worldMat); v[1].u = 1.0f; v[1].v = 0.0f;
    v[2].pos = VTransform(p3, worldMat); v[2].u = 0.0f; v[2].v = 1.0f;

    v[3].pos = v[1].pos;                 v[3].u = 1.0f; v[3].v = 0.0f;
    v[4].pos = VTransform(p4, worldMat); v[4].u = 1.0f; v[4].v = 1.0f;
    v[5].pos = v[2].pos;                 v[5].u = 0.0f; v[5].v = 1.0f;

    // 5. 描画
    DrawPolygon3D(v, 2, imageHandle_, TRUE);

}

// UIManager用座標取得
VECTOR Grid::GetWorldPosFromIndex(int index, bool isPlayerSide) {
    // 実装例（座標計算は適宜調整）
    float baseX = isPlayerSide ? 400.0f : 1200.0f;
    float baseY = 400.0f;
    float offset = 120.0f;
    return { baseX + index * offset, baseY, 0.0f };
}