#pragma once
#include "../AttackBase.h"

class IceAttack : public AttackBase {
public:
	IceAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter);
	~IceAttack() override = default;

	void Update() override;
	void Draw() override;
	void DrawWarning() override; // 追加: 抽象メソッドの実装宣言
	void Execute() override; // 衝突時に呼ばれる想定

	BulletType GetBulletType() const override { return BulletType::PLAYER; } // 追加
};