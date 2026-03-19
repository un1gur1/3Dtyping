#include "SwordAttack.h"
#include <DxLib.h>
#include <cmath>
#include "../../../Common/EffectManager.h" 

SwordAttack::SwordAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter, float delayTimer_)
	: AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter, delayTimer_)
{
	// ==========================================================
	// ★ 剣の斬撃エフェクトをロード
	// （パスは実際のファイル名に合わせて書き換えてください）
	// ==========================================================
	EffectManager::GetInstance().Load("sword", "Data/Image/efe3/ken.efk");

	// 近接攻撃用の予告エフェクトがあればここでロードしておくと便利です
	// EffectManager::GetInstance().Load("sword_warn", "Data/Image/efe2/warn.efk");
}

void SwordAttack::Update() {
	if (!isAlive_) return;

	// ==========================================
	// 1. ディレイ（予兆）待機：時間が来るまでは何もしない！
	// ==========================================
	if (delayTimer_ > 0.0f) {
		if (!warningPlayed_) {
			// ★ 敵が使った時など、必要ならここで予告エフェクトを再生
			// int ph = EffectManager::GetInstance().Play("sword_warn", pos_);
			warningPlayed_ = true;
		}

		delayTimer_ -= 1.0f / 60.0f;
		return; // 時間が来るまでは移動も当たり判定もさせない！
	}

	// ==========================================
	// 2. タイマーゼロ：本番の斬撃を発動！
	// ==========================================
	if (!attackExecuted_) {
		Execute();
		attackExecuted_ = true;
	}

	// ==========================================
	// 3. 親クラスのUpdate（移動や寿命の管理）
	// ==========================================
	AttackBase::Update(); // これを呼ぶと pos_ が velocity に従って移動する

	// ==========================================
	// 4. エフェクトを斬撃波と一緒に移動させる！
	// ==========================================
	if (effectPlayingId_ != -1) {
		// 弾（pos_）が移動したら、エフェクトも同じ場所に追従させる
		EffectManager::GetInstance().SetPos(effectPlayingId_, pos_);
	}

	// ※ 近接なので当たり判定はAttackManager等のブロードフェーズで処理される想定
}

void SwordAttack::Draw() {
	// エフェクトマネージャーがリッチなエフェクトを描画してくれるので、
	// デバッグ用の線が不要になればコメントアウトしてOKです！

	/*
	VECTOR p1 = pos_;
	VECTOR p2 = pos_;
	p2.x += 30.0f; // 向きに合わせて調整
	DrawLine3D(p1, p2, GetColor(220, 220, 255));
	*/
}

void SwordAttack::DrawWarning() {
	// 昔のワーニング表示（XZ平面の十字描画）
	// 今後はエフェクトマネージャーに任せる方針で進めるならコメントアウトでOKです！

	/*
	const float r = 60.0f;
	VECTOR p = pos_;
	VECTOR a = { p.x - r, p.y, p.z - r };
	VECTOR b = { p.x + r, p.y, p.z + r };
	VECTOR c = { p.x - r, p.y, p.z + r };
	VECTOR d = { p.x + r, p.y, p.z - r };
	int col = GetColor(255, 120, 120);
	DrawLine3D(a, b, col);
	DrawLine3D(c, d, col);
	*/
}

void SwordAttack::Execute() {
	// ==========================================================
	// ★ ここで「sword」エフェクトを再生！
	// ==========================================================
	effectPlayingId_ = EffectManager::GetInstance().Play("sword", pos_);

	// エフェクトの大きさを調整（見た目に合わせて数値をいじってください）
	EffectManager::GetInstance().SetScale(effectPlayingId_, 30.0f);

	// 【おまけ演出】飛んでいく方向（vel_）に合わせてエフェクトを回転させる
	if (vel_.x != 0.0f || vel_.z != 0.0f) {
		float angleY = atan2f(vel_.x, vel_.z);
		// エフェクトのY軸を回転させて、斬撃の向きを進行方向に合わせる
		EffectManager::GetInstance().SetRotation(effectPlayingId_, VGet(0.0f, angleY, 0.0f));
	}
}