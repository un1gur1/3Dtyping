#include "DarkAttack.h"
#include <DxLib.h>
#include <cmath>
#include "../../../Common/EffectManager.h" 

DarkAttack::DarkAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTime)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter, delayTime)
{
	EffectManager::GetInstance().Load("dark", "Data/Image/efe3/dark.efk");
}

void DarkAttack::Update() {
	if (!isAlive_) return;

	// 1. 予兆待機
	if (delayTimer_ > 0.0f) {
		if (!warningPlayed_) warningPlayed_ = true;
		delayTimer_ -= 1.0f / 60.0f;
		return;
	}

	// 2. 本発動
	if (!attackExecuted_) {
		Execute();
		attackExecuted_ = true;
	}

	// 3. 移動
	AttackBase::Update();

	// 4. エフェクト追従
	if (effectPlayingId_ != -1) {
		EffectManager::GetInstance().SetPos(effectPlayingId_, pos_);
	}
}

void DarkAttack::Draw() {}
void DarkAttack::DrawWarning() {}

void DarkAttack::Execute() {
	effectPlayingId_ = EffectManager::GetInstance().Play("dark", pos_);
	EffectManager::GetInstance().SetScale(effectPlayingId_, 35.0f); // 闇は少し大きく

	// 飛んでいく方向へ回転
	if (vel_.x != 0.0f || vel_.z != 0.0f) {
		float angleY = atan2f(vel_.x, vel_.z);
		EffectManager::GetInstance().SetRotation(effectPlayingId_, VGet(0.0f, angleY, 0.0f));
	}
}