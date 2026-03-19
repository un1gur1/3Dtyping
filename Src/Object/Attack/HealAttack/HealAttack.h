#pragma once
#include "../AttackBase.h"
#include "../../Actor/ActorBase.h" // shooter_ の情報を使うために必要

class HealAttack : public AttackBase {
public:
	// damage 引数は回復量として扱う
	HealAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int healAmount, ActorBase* shooter, float delayTime = 0.0f);
	~HealAttack() override = default;

	void Update() override;
	void Draw() override;
	void DrawWarning() override;
	void Execute() override;

	// 敵が回復を使うことも想定
	BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

private:
	// ★ エフェクト・遅延管理用の変数
	bool warningPlayed_ = false;
	bool attackExecuted_ = false;
	int effectPlayingId_ = -1; // 再生中の回復エフェクトを操るためのID
};