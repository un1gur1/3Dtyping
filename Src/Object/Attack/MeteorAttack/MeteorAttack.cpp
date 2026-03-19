#include "MeteorAttack.h"
#include <DxLib.h>
#include <cmath>
#include "../../../Common/EffectManager.h" 

MeteorAttack::MeteorAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTime)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter, delayTime)
	, fallSpeed_(0.0f)
{
	// 上空から降らせるため、生成時にY座標を高くしておく
	pos_.y = 1000.0f;

	// 初速（下向き）
	fallSpeed_ = velocity.y < 0 ? velocity.y : -100.0f;

	// 落下用エフェクトと、爆発用エフェクトの2つをロード！
	EffectManager::GetInstance().Load("meteor", "Data/Image/efe3/meteo.efk");
	EffectManager::GetInstance().Load("explosion", "Data/Image/efe3/explosion.efk");
}

void MeteorAttack::Update() {
	if (!isAlive_) return;

	// 1. 予兆（詠唱）待機
	if (delayTimer_ > 0.0f) {
		if (!warningPlayed_) {
			// 必要ならここで地面（pos_.x, 0.0f, pos_.z）に予告エフェクトを出す
			warningPlayed_ = true;
		}
		delayTimer_ -= 1.0f / 60.0f;
		return;
	}

	// 2. 落下開始（ここでメテオの球体エフェクトを出す）
	if (!attackExecuted_) {
		effectPlayingId_ = EffectManager::GetInstance().Play("meteor", pos_);
		EffectManager::GetInstance().SetScale(effectPlayingId_, 300.0f);
		attackExecuted_ = true;
	}

	// 3. 落下運動の処理
	if (!impacted_) {
		fallSpeed_ -= 15.0f; // 重力でどんどん加速（下向き）
		pos_.y += fallSpeed_ * (1.0f / 60.0f);

		// 斜めに落としたい場合は X や Z も動かす
		pos_.x += vel_.x * (1.0f / 60.0f);
		pos_.z += vel_.z * (1.0f / 60.0f);

		// エフェクトを落下する隕石に追従させる
		if (effectPlayingId_ != -1) {
			EffectManager::GetInstance().SetPos(effectPlayingId_, pos_);
		}

		// 地面（Y=0）に到達したらドカーン！！
		if (pos_.y <= 0.0f) {
			pos_.y = 0.0f;
			Execute();
		}
	}

	// 寿命が来たら消滅
	lifeTime_ -= 1.0f / 60.0f;
	if (lifeTime_ <= 0.0f) Kill();
}

void MeteorAttack::Draw() {}
void MeteorAttack::DrawWarning() {}

void MeteorAttack::Execute() {
	if (impacted_) return;
	impacted_ = true;

	// 落下中のメテオエフェクトを消す
	if (effectPlayingId_ != -1) {
		EffectManager::GetInstance().Stop(effectPlayingId_);
	}

	// 代わりに大爆発エフェクトを再生！
	int expId = EffectManager::GetInstance().Play("explosion", pos_);
	EffectManager::GetInstance().SetScale(expId, 100.0f); // 爆発は巨大に！

	// ※ 周囲へのダメージ判定などは AttackManager 側で行われる想定
}