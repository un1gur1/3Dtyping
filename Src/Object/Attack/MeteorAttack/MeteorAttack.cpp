#include "MeteorAttack.h"
#include <DxLib.h>
#include <cmath>

MeteorAttack::MeteorAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
	, fallSpeed_(0.5f * (velocity.y + -200.0f + std::fabs(velocity.y - -200.0f)))
{
	// 初期Yを上空にする場合は生成時に pos_.y を上げるなど Scene 側で調整
}

void MeteorAttack::Update() {
	// 落下運動の簡易実装
	fallSpeed_ += 9.8f * (1.0f / 60.0f); // 重力疑似
	pos_.y += fallSpeed_ * (1.0f / 60.0f);
	AttackBase::Update();

	// 地面到達や寿命で衝突判定→Execute を呼ぶ処理がある想定
}

void MeteorAttack::Draw() {
	// 火の尾を表現
	DrawSphere3D(pos_, 28.0f, 12, GetColor(255, 160, 60), GetColor(255, 80, 10), TRUE);
	// 簡単な尾を線で描く
	DrawLine3D({ pos_.x, pos_.y + 10.0f, pos_.z }, { pos_.x - fallSpeed_ * 0.02f, pos_.y + 30.0f, pos_.z }, GetColor(255, 120, 60));
}

void MeteorAttack::DrawWarning() {
	// ワーニング表示：衝突範囲を示す赤い十字
	const float r = 90.0f;
	VECTOR p = pos_;
	VECTOR a = { p.x - r, p.y, p.z - r };
	VECTOR b = { p.x + r, p.y, p.z + r };
	VECTOR c = { p.x - r, p.y, p.z + r };
	VECTOR d = { p.x + r, p.y, p.z - r };
	int col = GetColor(255, 120, 60);
	DrawLine3D(a, b, col);
	DrawLine3D(c, d, col);
}

void MeteorAttack::Execute() {
	if (impacted_) return;
	impacted_ = true;
	// 広域ダメージ、エフェクト発生などはここで行う（AttackManagerへ通知等）
	// この攻撃はヒット時に消滅する想定
}