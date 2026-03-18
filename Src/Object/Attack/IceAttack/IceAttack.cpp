#include "IceAttack.h"
#include <DxLib.h>

IceAttack::IceAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
	// 氷は見た目を少し透き通らせたい等の初期化があればここで
}

void IceAttack::Update() {
	AttackBase::Update();
	// 必要なら減速や持続効果のカウントなどをここで行う
}

void IceAttack::Draw() {
	// 氷らしい色で球体を描画
	DrawSphere3D(pos_, 22.0f, 12, GetColor(160, 220, 255), GetColor(220, 240, 255), TRUE);

	// ひとつまみの氷の欠片を線で表現
	VECTOR p1 = pos_;
	VECTOR p2 = pos_;
	p2.x += 18.0f;
	DrawLine3D(p1, p2, GetColor(200, 230, 255));
	VECTOR q1 = pos_;
	VECTOR q2 = pos_;
	q2.z += 14.0f;
	DrawLine3D(q1, q2, GetColor(180, 215, 255));
}

void IceAttack::DrawWarning() {
	// ワーニング表示：薄いシアンの十字
	const float r = 70.0f;
	VECTOR p = pos_;
	VECTOR a = { p.x - r, p.y, p.z };
	VECTOR b = { p.x + r, p.y, p.z };
	VECTOR c = { p.x, p.y, p.z - r };
	VECTOR d = { p.x, p.y, p.z + r };
	int col = GetColor(160, 220, 255);
	DrawLine3D(a, b, col);
	DrawLine3D(c, d, col);
}

void IceAttack::Execute() {
	// 衝突時の処理：ダメージ付与などは AttackManager 側で処理される想定。
	// この攻撃は当たったら消えるようにする（必要ならフリーズ付与などをここで呼ぶ）
	Kill();
}