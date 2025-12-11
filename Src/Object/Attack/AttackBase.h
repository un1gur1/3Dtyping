#pragma once
#include <DxLib.h>

class AttackBase {
public:
    AttackBase();
    virtual ~AttackBase();

    // 毎フレーム更新
    virtual void Update() = 0;
    // 描画
    virtual void Draw() = 0;

    // 位置取得・設定
    const VECTOR& GetPos() const { return pos_; }
    void SetPos(const VECTOR& pos) { pos_ = pos; }

    // ダメージ値取得
    int GetDamage() const { return damage_; }

    // 生存フラグ
    bool IsAlive() const { return isAlive_; }
    void Kill() { isAlive_ = false; }

protected:
    VECTOR pos_;      // 位置
    VECTOR vel_;      // 速度
    int damage_;      // ダメージ
    bool isAlive_;    // 生存フラグ
};
