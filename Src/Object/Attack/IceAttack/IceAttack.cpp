#include "IceAttack.h"
#include <DxLib.h>
#include <cmath>
#include "../../../Common/EffectManager.h" 

IceAttack::IceAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTime)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter, delayTime)
{
	// ==========================================================
	// ★ 氷エフェクトをロード
	// （パスは実際のファイル名に合わせて書き換えてください）
	// ==========================================================
	EffectManager::GetInstance().Load("ice", "Data/Image/efe3/cold.efk");
}

void IceAttack::Update() {
	if (!isAlive_) return;

	// ==========================================
	// 1. ディレイ（予兆）待機：時間が来るまでは何もしない！
	// ==========================================
	if (delayTimer_ > 0.0f) {
		if (!warningPlayed_) {
			// 必要ならここで氷の予告エフェクトを再生
			warningPlayed_ = true;
		}
		delayTimer_ -= 1.0f / 60.0f;
		return;
	}

	// ==========================================
	// 2. タイマーゼロ：本番の氷魔法を発動！
	// ==========================================
	if (!attackExecuted_) {
		Execute();
		attackExecuted_ = true;
	}

	// ==========================================
	// 3. 親クラスのUpdate（移動や寿命の管理）
	// ==========================================
	AttackBase::Update();

	// ==========================================
	// 4. エフェクトを氷の弾と一緒に移動させる！
	// ==========================================
	if (effectPlayingId_ != -1) {
		EffectManager::GetInstance().SetPos(effectPlayingId_, pos_);
	}
}

void IceAttack::Draw() {
	// エフェクトマネージャーが描画してくれるので空でOK！
}

void IceAttack::DrawWarning() {
	// エフェクトマネージャーに任せるので空でOK！
}

void IceAttack::Execute() {
	// ==========================================================
	// ★ ここで「ice」エフェクトを再生！
	// ==========================================================
	effectPlayingId_ = EffectManager::GetInstance().Play("ice", pos_);
	EffectManager::GetInstance().SetScale(effectPlayingId_, 30.0f); // 氷の大きさ

	// 【おまけ演出】飛んでいく方向（vel_）に合わせてエフェクトを回転
	if (vel_.x != 0.0f || vel_.z != 0.0f) {
		float angleY = atan2f(vel_.x, vel_.z);
		EffectManager::GetInstance().SetRotation(effectPlayingId_, VGet(0.0f, angleY, 0.0f));
	}
}