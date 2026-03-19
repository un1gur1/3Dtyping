#pragma once
#include "../AttackBase.h"
#include "../../Actor/ActorBase.h"
#include "../../../Common/UiManager.h"
#include <vector>

class ThunderAttack : public AttackBase
{
public:
	// コンストラクタ（遅延タイマー対応済み）
	ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTimer_ = 0.0f);

	void Update() override;
	void Draw() override;
	void DrawWarning() override;
	void Execute() override;

	BulletType GetBulletType() const override { return isPlayer_ ? BulletType::PLAYER : BulletType::ENEMY; }

	void SetTargets(const std::vector<ActorBase*>& targets) {
		targets_ = targets;
	}

private:
	struct ThunderBullet {
		VECTOR pos;
		VECTOR vel;
		bool isActive = true;
		int gridIndex;
		float elapsed = 0.0f;
		bool hasDealtDamage = false; // 追加：エフェクト再生時にダメージを与えたか
	};

	std::vector<ThunderBullet> bullets_; // 発射された弾

	// 遅延と状態管理
	bool warningPlayed_ = false;   // 予告エフェクトを再生したか
	bool attackExecuted_ = false;  // 本番の雷を落としたか
	void DetermineStrikePositions(); // 落下地点を事前に計算する関数

	float bulletLifeTime_ = 1.0f; // 弾の寿命

	// 生成する落雷スポット
	std::vector<VECTOR> strikePositions_;
	std::vector<int> strikeGridIndices_;
	int strikeCount_ = 3;
	std::vector<ActorBase*> targets_;
};