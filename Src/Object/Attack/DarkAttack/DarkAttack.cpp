#include "DarkAttack.h"
#include <DxLib.h>

DarkAttack::DarkAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
}

void DarkAttack::Update() {
	AttackBase::Update();
	// 闇らしい揺らぎや追尾ロジックを追加可能
}

void DarkAttack::Draw() {
	// 暗めの紫色で表現
	DrawSphere3D(pos_, 24.0f, 12, GetColor(80, 40, 120), GetColor(120, 70, 160), TRUE);
}

void DarkAttack::Execute() {
	// 例: 通常ダメージに加えて視界低下などの副作用を与えるフックを呼ぶ
}