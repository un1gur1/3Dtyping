#include "HealAttack.h"
#include <DxLib.h>

HealAttack::HealAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int healAmount, ActorBase* shooter)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, healAmount, shooter)
{
}

void HealAttack::Update() {
	AttackBase::Update();
}

void HealAttack::Draw() {
	// 緑っぽい光で回復を表現
	DrawSphere3D(pos_, 20.0f, 12, GetColor(120, 255, 160), GetColor(180, 255, 200), TRUE);
}

void HealAttack::DrawWarning() {
	// ワーニング表示：緑の円または十字
	const float r = 60.0f;
	VECTOR p = pos_;
	VECTOR a = { p.x - r, p.y, p.z - r };
	VECTOR b = { p.x + r, p.y, p.z + r };
	VECTOR c = { p.x - r, p.y, p.z + r };
	VECTOR d = { p.x + r, p.y, p.z - r };
	int col = GetColor(120, 220, 140);
	DrawLine3D(a, b, col);
	DrawLine3D(c, d, col);
}

void HealAttack::Execute() {
	// 発射者を回復する例
	if (shooter_) {
		// ApplyDamage に負の値を渡して回復する方式を想定
		shooter_->ApplyDamage(-GetDamage()); // GetDamage() が無ければ damage を直接使う設計に合わせる
	}
	// 回復エフェクトや消滅処理をここで行う
}