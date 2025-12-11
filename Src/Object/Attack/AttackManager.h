#pragma once
#include <vector>
#include "AttackBase.h"

class AttackManager {
public:
    AttackManager();
    ~AttackManager();

    // 攻撃の追加
    void Add(AttackBase* attack);

    // 全攻撃の更新・描画
    void UpdateAll();
    void DrawAll();

    // 全攻撃の取得
    const std::vector<AttackBase*>& GetAttacks() const { return attacks_; }

    // 全削除
    void Clear();

private:
    std::vector<AttackBase*> attacks_; // 生ポインタで管理
};