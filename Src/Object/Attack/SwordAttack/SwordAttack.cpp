#include "SwordAttack.h"
#include <DxLib.h>

SwordAttack::SwordAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
	// 近接なので短い寿命・小さい飛距離が期待される
}

void SwordAttack::Update() {
	AttackBase::Update();
	// 近接なので当たり判定は即時処理される想定
}

void SwordAttack::Draw() {
	// 剣っぽい表現：短い線分を描画
	VECTOR p1 = pos_;
	VECTOR p2 = pos_;
	p2.x += 30.0f; // 向きに合わせて調整してください
	DrawLine3D(p1, p2, GetColor(220, 220, 255));
}

void SwordAttack::DrawWarning() {
	// ワーニング表示（XZ平面に十字を描画）
	const float r = 60.0f;
	VECTOR p = pos_;
	VECTOR a = { p.x - r, p.y, p.z - r };
	VECTOR b = { p.x + r, p.y, p.z + r };
	VECTOR c = { p.x - r, p.y, p.z + r };
	VECTOR d = { p.x + r, p.y, p.z - r };
	int col = GetColor(255, 120, 120);
	DrawLine3D(a, b, col);
	DrawLine3D(c, d, col);
}

void SwordAttack::Execute() {
	// 近接ヒットでダメージを与える処理（衝突先 Actor* があればそちらへ ApplyDamage）
}