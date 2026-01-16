#include "UltimateAttack.h"
#include <DxLib.h>

UltimateAttack::UltimateAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{


}

void UltimateAttack::Update() {
    AttackBase::Update();
    // 必要なら追加処理
}

void UltimateAttack::Draw() {
    DrawSphere3D(pos_, 30.0f, 16, GetColor(255, 100, 100), GetColor(200, 122, 15), true);
}

void UltimateAttack::DrawWarning() {
    // 必要に応じて実装
}

void UltimateAttack::Execute() {
    // 必要に応じて実装
}
