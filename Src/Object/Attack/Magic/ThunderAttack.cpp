#include "ThunderAttack.h"
#include "../../../Common/Grid.h"
#include <DxLib.h>
#include <random>
#include <algorithm>
#include "../../Actor/Player/Player.h"

ThunderAttack::ThunderAttack(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : AttackBase(targetGridIdx, isPlayer, velocity, lifeTime, damage, shooter)
{
}

void ThunderAttack::Update()
{
    if (!isAlive_) return;

    elapsed_ += 1.0f / 60.0f;

    // 予兆中は予兆表示のみ（AttackManager 側で DrawWarning を呼ぶ設計に合わせて、ここでも呼ぶ）
    if (!executed_ && elapsed_ < warningTime_) {
        DrawWarning();
        return;
    }

    if (!executed_) {
        Execute(); // 予兆終了後に弾発射（位置は事前に決定済）
        executed_ = true;
    }

    // 弾の移動・寿命管理（自由落下）
    for (auto& bullet : bullets_) {
        if (!bullet.isActive) continue;
        bullet.pos.y += bullet.vel.y * (1.0f / 60.0f);
        bullet.elapsed += 1.0f / 60.0f;
        if (bullet.elapsed > bulletLifeTime_) bullet.isActive = false;
    }

    // 全弾消滅で生存フラグを落とす
    bool anyActive = false;
    for (const auto& bullet : bullets_) {
        if (bullet.isActive) anyActive = true;
    }
    if (!anyActive) {
        isAlive_ = false;
    }
}

void ThunderAttack::Draw()
{
    // 弾を描画（弾ごとの位置で描く）
    for (const auto& bullet : bullets_) {
        if (!bullet.isActive) continue;
        DrawSphere3D(bullet.pos, 30.0f, 16, GetColor(255, 200, 50), GetColor(255, 200, 50), true);
        // 地面エフェクト（小さい円）を描く
        VECTOR ground = bullet.pos;
        ground.y = 0.0f;
        DrawSphere3D(ground, 60.0f, 24, GetColor(255, 180, 80), GetColor(0, 0, 0), true);
    }
}

void ThunderAttack::DrawWarning()
{
    // 予兆時に strikePositions_ が空なら一度だけ決める
    if (strikePositions_.empty()) {
        // 主要ターゲット: pos_ がセットされていればそれを使い、未設定なら targetGridIdx_ から算出、さらにそれもなければ shooter の位置
        VECTOR primary = pos_;
        bool hasPrimary = !(primary.x == 0.0f && primary.y == 0.0f && primary.z == 0.0f);
        if (!hasPrimary) {
            primary = Grid::GetWorldPosFromIndex(targetGridIdx_, isPlayer_);
            hasPrimary = true;
        }
        if (!hasPrimary && shooter_) {
            primary = shooter_->GetPos();
        }

        // 追加ランダムスポットを選ぶ（グリッド交点 -800..800 step 400）
        std::mt19937 mt{ static_cast<unsigned int>(std::random_device{}()) };
        std::uniform_int_distribution<int> dist(-2, 2); // -2,-1,0,1,2 -> multiplied by 400 gives -800..800

        // primary を先頭に格納
        strikePositions_.push_back(primary);
        strikeGridIndices_.push_back(AttackBase::CalcGridIndex(primary, isPlayer_));

        // ランダムで strikeCount_-1 個追加（重複回避）
        int attempts = 0;
        while ((int)strikePositions_.size() < strikeCount_ && attempts < 32) {
            int gx = dist(mt) * 400;
            int gz = dist(mt) * 400;
            VECTOR p = { static_cast<float>(gx), 0.0f, static_cast<float>(gz) };
            // 重複回避（同じグリッドが選ばれたらスキップ）
            int gidx = AttackBase::CalcGridIndex(p, isPlayer_);
            bool dup = false;
            for (int existing : strikeGridIndices_) if (existing == gidx) { dup = true; break; }
            if (!dup) {
                strikePositions_.push_back(p);
                strikeGridIndices_.push_back(gidx);
            }
            attempts++;
        }
    }

    // UI のグリッドハイライトを Warning に設定
    for (int idx : strikeGridIndices_) {
        UIManager::GetInstance().SetGridState(idx, Grid::GridState::Warning, isPlayer_);
    }
}

void ThunderAttack::Execute()
{
    // グリッド状態を Attack に変更して、上空から落とす弾を生成
    for (size_t i = 0; i < strikePositions_.size(); ++i) {
        const VECTOR& target = strikePositions_[i];
        int gridIdx = strikeGridIndices_[i];
        UIManager::GetInstance().SetGridState(gridIdx, Grid::GridState::Attack, isPlayer_);

        ThunderBullet bullet;
        // 上空から落とす（高さはランダム化して見栄えを付ける）
        bullet.pos = target;
        bullet.pos.y = 300.0f + (float)(i * 40); // 少しだけ高さをずらす
        bullet.vel = { 0.0f, -600.0f, 0.0f }; // 下方向
        bullet.gridIndex = gridIdx;
        bullet.isActive = true;
        bullet.elapsed = 0.0f;
        bullets_.push_back(bullet);
    }
}