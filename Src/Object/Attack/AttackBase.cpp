#include "AttackBase.h"
#include "../../Common/UiManager.h"
#include "../../Common/Grid.h"
// コンストラクタ
AttackBase::AttackBase(int targetGridIdx, bool isPlayer, const VECTOR& velocity, float lifeTime, int damage, ActorBase* shooter)
    : targetGridIdx_(targetGridIdx), isPlayer_(isPlayer), vel_(velocity), lifeTime_(lifeTime), damage_(damage), isAlive_(true), shooter_(shooter)
{
    // グリッド中心座標から初期位置を取得
    pos_ = Grid::GetWorldPosFromIndex(targetGridIdx_,isPlayer_);
    // 攻撃開始時にグリッドを攻撃状態に
    UIManager::GetInstance().SetGridState(targetGridIdx_, Grid::GridState::Attack, isPlayer_);
}

// デストラクタ
AttackBase::~AttackBase()
{
    // 攻撃終了時にグリッド状態をリセット
    UIManager::GetInstance().SetGridState(targetGridIdx_, Grid::GridState::Normal, isPlayer_);
}

// 共通の更新処理
void AttackBase::Update()
{
    if (!isAlive_) return;
    // 移動
    pos_.x += vel_.x;
    pos_.y += vel_.y;
    pos_.z += vel_.z;
    // 寿命
    lifeTime_ -= 1.0f / 60.0f;
    if (lifeTime_ <= 0.0f) {
        isAlive_ = false;
    }
    // 攻撃中はグリッドを攻撃状態に
    UIManager::GetInstance().SetGridState(targetGridIdx_, Grid::GridState::Attack, isPlayer_);
}
