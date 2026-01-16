#include "RangedAttack.h"
#include <DxLib.h>

RangedAttack::RangedAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{


}

void RangedAttack::Update() {
    AttackBase::Update();
    // 必要なら追加処理
}

void RangedAttack::Draw() {
    DrawSphere3D(pos_, 30.0f, 16, GetColor(255, 100, 100), GetColor(255, 100, 100), true);
}

void RangedAttack::DrawWarning() {
    // 例：グリッドに警告エフェクトを出す場合
     UIManager::GetInstance().SetGridState(targetGridIdx_, Grid::GridState::Warning, isPlayer_);
}

void RangedAttack::Execute() {
    // 例：着弾時の追加処理など
}
