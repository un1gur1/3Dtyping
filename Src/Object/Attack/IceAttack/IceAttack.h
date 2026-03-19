#pragma once
#include "../AttackBase.h"

class IceAttack : public AttackBase {
public:
	// コンストラクタ（遅延タイマー対応）
	IceAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTime = 0.0f);
	~IceAttack() override = default;

	void Update() override;
	void Draw() override;
	void DrawWarning() override;
	void Execute() override;

	// 敵が撃ったかどうかも判定できるように変更
	BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

private:
	// ★ エフェクト・遅延管理用の変数を追加
	bool warningPlayed_ = false;
	bool attackExecuted_ = false;
	int effectPlayingId_ = -1; // 再生中の氷エフェクトを操るためのID
};