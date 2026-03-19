#include "HealAttack.h"
#include <DxLib.h>
#include "../../../Common/EffectManager.h" 

HealAttack::HealAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int healAmount, ActorBase* shooter, float delayTime)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, healAmount, shooter, delayTime)
{
	// ==========================================================
	// ★ 回復エフェクトをロード
	// （パスは実際のファイル名に合わせて書き換えてください）
	// ==========================================================
	EffectManager::GetInstance().Load("heal", "Data/Image/efe3/heal.efk");
}

void HealAttack::Update() {
	if (!isAlive_) return;

	// ==========================================
	// ★ 最大の特徴：回復は常に術者（キャラクター）の位置に追従する！
	// ==========================================
	if (shooter_) {
		pos_ = shooter_->GetPos();
	}

	// ==========================================
	// 1. ディレイ（詠唱）待機
	// ==========================================
	if (delayTimer_ > 0.0f) {
		if (!warningPlayed_) {
			// 必要なら足元に魔法陣などの予告エフェクトを出す
			warningPlayed_ = true;
		}
		delayTimer_ -= 1.0f / 60.0f;
		return;
	}

	// ==========================================
	// 2. タイマーゼロ：本番の回復発動！
	// ==========================================
	if (!attackExecuted_) {
		Execute();
		attackExecuted_ = true;
	}

	// ==========================================
	// 3. 親クラスのUpdate（寿命管理など）
	// ※ 速度(velocity)は0の想定なので勝手には飛んでいきません
	// ==========================================
	AttackBase::Update();

	// ==========================================
	// 4. エフェクトをキャラと一緒に移動させる
	// ==========================================
	if (effectPlayingId_ != -1) {
		EffectManager::GetInstance().SetPos(effectPlayingId_, pos_);
	}
}

void HealAttack::Draw() {
	// エフェクトマネージャーに任せるので、昔の緑の球体描画は削除！
}

void HealAttack::DrawWarning() {
	// 昔の緑の十字描画も削除！
}

void HealAttack::Execute() {
	// ==========================================================
	// ★ ここで「heal」エフェクトを再生！
	// ==========================================================
	effectPlayingId_ = EffectManager::GetInstance().Play("heal", pos_);

	// キャラクターを包み込むような大きさに調整（見た目に合わせて変更してください）
	EffectManager::GetInstance().SetScale(effectPlayingId_, 30.0f);

	// ==========================================================
	// ★ 実際の回復処理
	// ==========================================================
	if (shooter_) {
		// ApplyDamage にマイナスの値を渡すことで「回復」扱いにする設計
		shooter_->ApplyDamage(-damage_);
	}
}