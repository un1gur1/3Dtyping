#include "ThunderAttack.h"
#include "../../../Common/Grid.h"
#include <DxLib.h>
#include <random>
#include <algorithm>
#include "../../Actor/Player/Player.h"
#include "../../Actor/ActorBase.h"
#include "../../../Application.h"
#include "../../../Common/UiManager.h"
#include "../../../Common/EffectManager.h" 

ThunderAttack::ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTimer_)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter, delayTimer_)
{
	// ==========================================================
	// ★ ここでエフェクトをロード！
	// （※EffectManagerは何度呼ばれても1回しかロードしないので安全です）
	// ==========================================================
	EffectManager::GetInstance().Load("thunder_warn", "Data/Image/efe2/thun.efk");
	EffectManager::GetInstance().Load("thunder_main", "Data/Image/efe2/efe.efk");
}

void ThunderAttack::DetermineStrikePositions()
{
	strikePositions_.clear();
	strikeGridIndices_.clear();

	if (!isPlayer_) {
		strikePositions_.push_back(pos_);
		strikeGridIndices_.push_back(AttackBase::CalcGridIndex(pos_, isPlayer_));
	}
	else {
		for (auto* a : targets_) {
			if (!a || !a->IsEnemy()) continue;
			VECTOR p = a->GetPos();
			p.y += 150.0f;
			int gidx = AttackBase::CalcGridIndex(p, false);
			strikePositions_.push_back(p);
			strikeGridIndices_.push_back(gidx);
		}
		if (strikePositions_.empty()) {
			VECTOR fallback = pos_;
			if (fallback.x == 0.0f && fallback.y == 0.0f && fallback.z == 0.0f) {
				fallback = Grid::GetWorldPosFromIndex(targetGridIdx_, isPlayer_);
			}
			strikePositions_.push_back(fallback);
			strikeGridIndices_.push_back(AttackBase::CalcGridIndex(fallback, isPlayer_));
		}
	}
}

void ThunderAttack::Update()
{
	if (!isAlive_) return;

	// ==========================================
	// 1. 生成直後：落下地点を決定し、予告エフェクトを出す
	// ==========================================
	if (!warningPlayed_) {
		DetermineStrikePositions();

		// ディレイ（詠唱時間）がある場合は予告を出す
		if (delayTimer_ > 0.0f) {
			for (const auto& target : strikePositions_) {
				// ★ マネージャーを使ってたった2行で予告エフェクト再生！
				int ph = EffectManager::GetInstance().Play("thunder_warn", target);
				EffectManager::GetInstance().SetScale(ph, 50.0f);
			}
		}
		warningPlayed_ = true;
	}

	// ==========================================
	// 2. ディレイ（予兆）待機：時間が来るまでは何もしない！
	// ==========================================
	if (delayTimer_ > 0.0f) {
		delayTimer_ -= 1.0f / 60.0f;
		return;
	}

	// ==========================================
	// 3. タイマーゼロ：本番の雷を発射！
	// ==========================================
	if (!attackExecuted_) {
		Execute();
		attackExecuted_ = true;
	}

	// ==========================================
	// 4. 以降は弾の移動・ダメージ処理
	// ==========================================
	for (auto& bullet : bullets_) {
		if (!bullet.isActive) continue;
		bullet.pos.y += bullet.vel.y * (1.0f / 60.0f);
		bullet.elapsed += 1.0f / 60.0f;

		if (bullet.isActive && bullet.pos.y <= 0.0f) {
			for (auto* tgt : targets_) {
				if (!tgt || !tgt->GetisCollision()) continue;
				const VECTOR tpos = tgt->GetPos();
				float dx = tpos.x - bullet.pos.x;
				float dy = tpos.y - bullet.pos.y;
				float dz = tpos.z - bullet.pos.z;
				float distSq = dx * dx + dy * dy + dz * dz;
				float radiusSum = 100.0f + tgt->GetCapsuleRadius();
				if (distSq < radiusSum * radiusSum) {
					tgt->ApplyDamage(damage_);
					Application::GetInstance()->ShakeScreen(5, 30, true, true);
				}
			}
			bullet.isActive = false;
			bullet.elapsed = 0.0f;
		}
		if (bullet.elapsed > bulletLifeTime_) bullet.isActive = false;
	}

	bool anyActive = false;
	for (const auto& bullet : bullets_) {
		if (bullet.isActive) anyActive = true;
	}
	if (!anyActive) isAlive_ = false;
}

void ThunderAttack::Draw() {
	// （描画はエフェクトに任せるのでここは空でもOKです）
}

void ThunderAttack::DrawWarning() {}

void ThunderAttack::Execute()
{
	for (size_t i = 0; i < strikePositions_.size(); ++i) {
		const VECTOR& target = strikePositions_[i];
		int gridIdx = strikeGridIndices_[i];
		UIManager::GetInstance().SetGridState(gridIdx, Grid::GridState::Attack, isPlayer_);

		ThunderBullet bullet;
		bullet.pos = target;
		bullet.pos.y = 300.0f;
		bullet.vel = { 0.0f, -600.0f, 0.0f };
		bullet.gridIndex = gridIdx;
		bullet.isActive = true;
		bullet.elapsed = 0.0f;
		bullets_.push_back(bullet);

		// ==========================================================
		// ★ 本番の落雷エフェクトもマネージャーにお任せ！
		// ==========================================================
		VECTOR groundPos = { target.x, 0.0f, target.z }; // 地面に落とす
		int ph = EffectManager::GetInstance().Play("thunder_main", groundPos);
		EffectManager::GetInstance().SetScale(ph, 100.0f);
	}
}