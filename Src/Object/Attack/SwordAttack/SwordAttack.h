#pragma once
#include "../AttackBase.h"

class SwordAttack : public AttackBase
{
public:
	// コンストラクタ（遅延タイマー対応）
	SwordAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTimer_ = 0.0f);

	void Update() override;
	void Draw() override;
	void DrawWarning() override;
	void Execute() override;

	BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

private:
	// 遅延とエフェクト管理用の変数
	bool warningPlayed_ = false;
	bool attackExecuted_ = false;
	int effectPlayingId_ = -1; // 再生中の剣エフェクトを操るためのID
};