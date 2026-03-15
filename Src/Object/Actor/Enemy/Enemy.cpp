#include "Enemy.h"
#include <random>

#include "../../Attack/AttackManager.h"
#include "../../Attack/AttackBase.h"
#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../Common/AnimationController.h"
#include "../../Attack/RangedAttack/RangedAttack.h"
#include "../../Attack/UltimateAttack/UltimateAttack.h"
#include "../Player/Player.h"
#include "../../Attack/Magic/ThunderAttack.h"
#include "../../../Common/UiManager.h"

// =======================================================
// エネミー専用のヘルパー関数（文字数カウント用）
// =======================================================
namespace {
	// UTF-8の文字列から「実際の文字数」をカウントする
	// （ひらがな1文字が3バイトになる問題を解決するため）
	static size_t GetUtf8CharCount(const std::string& s) {
		size_t count = 0;
		for (unsigned char c : s) {
			// UTF-8の先頭バイト（10xxxxxx 以外）のときだけカウント
			if ((c & 0xC0) != 0x80) {
				count++;
			}
		}
		return count;
	}

	// 先頭から n 文字分の UTF-8 部分文字列を返す（n は「文字数」単位）
	static std::string Utf8Substring(const std::string& s, size_t n) {
		if (n == 0 || s.empty()) return std::string();

		std::string out;
		out.reserve(s.size());
		size_t chars = 0;
		for (size_t i = 0; i < s.size(); ) {
			unsigned char c = static_cast<unsigned char>(s[i]);
			size_t len = 1;
			if (c < 0x80) len = 1;
			else if ((c & 0xE0) == 0xC0) len = 2;
			else if ((c & 0xF0) == 0xE0) len = 3;
			else if ((c & 0xF8) == 0xF0) len = 4;
			// 範囲チェック
			if (i + len > s.size()) len = s.size() - i;
			if (chars >= n) break;
			out.append(s.data() + i, len);
			i += len;
			chars++;
		}
		return out;
	}
}

// コンボセット初期化用ヘルパー（ローカル）
// （省略せず既存の内容を維持）
namespace {
	// シンプルなコンボを作成: 低HPでは重め・必殺を含むセットにする
	static std::vector<std::vector<Enemy::EnemyAction>> CreateDefaultCombos() {
		using A = Enemy::EnemyAction;
		std::vector<std::vector<A>> sets;
		// 高HP：安全に遠距離中心の軽いコンボ
		sets.push_back({ A::ATTACK_RANGE, A::ATTACK_RANGE, A::ATTACK_RANGE });
		// 中HP：遠距離＋近距離を混ぜる
		sets.push_back({ A::ATTACK_RANGE, A::THUNDER, A::ATTACK_RANGE });
		// 低HP：激しいコンボ、最後にULTIMATEへ繋げる可能性あり
		sets.push_back({ A::THUNDER, A::ATTACK_NEAR, A::THUNDER, A::ULTIMATE });
		return sets;
	}
}

Enemy::Enemy(Player* player)
{
	player_ = player;
	attackManager_ = nullptr;
	state_ = ActorState::IDLE;

	// コンボセット初期化
	comboSets_ = CreateDefaultCombos();
	activeComboIndex_ = -1;
	comboStep_ = 0;
	comboExecuting_ = false;
	comboActionInProgress_ = false;
}

Enemy::~Enemy(void)
{
}

void Enemy::Update(void)
{
	ActorBase::Update();

	// HP比率
	float hpRatio = (maxHp_ > 0) ? static_cast<float>(hp_) / static_cast<float>(maxHp_) : 1.0f;

	// コンボ実行中の進行管理
	if (comboExecuting_) {
		// 現在のアクションが未開始なら開始する
		if (!comboActionInProgress_) {
			if (activeComboIndex_ >= 0 && activeComboIndex_ < static_cast<int>(comboSets_.size())
				&& comboStep_ < static_cast<int>(comboSets_[activeComboIndex_].size())) {
				EnemyAction action = comboSets_[activeComboIndex_][comboStep_];
				// 次のアクションに応じて ActorState を設定（既存のステートで処理）
				switch (action) {
				case EnemyAction::ATTACK_RANGE:
					ChangeState(ActorState::ATTACK_RANGE);
					break;
				case EnemyAction::ATTACK_NEAR:
				case EnemyAction::THUNDER:
					// ATTACK_NEAR のコードで落雷や近接を扱うようにする
					ChangeState(ActorState::ATTACK_NEAR);
					break;
				case EnemyAction::ULTIMATE:
					ChangeState(ActorState::ULTIMATE);
					break;
				default:
					ChangeState(ActorState::IDLE);
					break;
				}
				// アクション開始をマーク（各ステート内部で attackRegistered_ が使われる）
				comboActionInProgress_ = true;
			}
			else {
				// コンボ終了
				comboExecuting_ = false;
				activeComboIndex_ = -1;
				comboStep_ = 0;
				comboActionInProgress_ = false;
				ChangeState(ActorState::IDLE);
			}
		}
		else {
			// アクション実行中 -> 既存ステート処理により攻撃が生成される
			// アクション完了判定: State が IDLE に戻り、attackRegistered_ が false になっているとき
			if (state_ == ActorState::IDLE && !comboActionInProgress_) {
				// （二重判定防止: ここ何もしない）
			}
			if (state_ == ActorState::IDLE && comboActionInProgress_ && !attackRegistered_) {
				// 現在のアクションが完了したと判断して次へ進める
				comboActionInProgress_ = false;
				comboStep_++;
				// コンボ終了チェックは次ループで行う
			}
		}
	}

	switch (state_) {
	case ActorState::IDLE:
		stateTimer_ -= 1.0f / 60.0f; // 60FPS想定
		moveTime_ += 0.5f / 60.0f; // フレームごとに加算
		//// 振幅200, 周期2秒で左右に動く
		pos_.x = 30.0f * sinf(moveTime_ * DX_TWO_PI / 2.0f);

		if (stateTimer_ <= 0.0f) {
			// HP閾値でULTIMATE強制遷移
			if (hp_ < maxHp_ * 0.5f) {
				ChangeState(ActorState::ULTIMATE);
				break;
			}

			// コンボ実行の開始: HP状態に応じたコンボを選択して順に実行する
			if (!comboExecuting_) {
				// HP状態ごとにコンボ選択
				int comboChoice = 0;
				if (hpRatio >= HP_HIGH) {
					// 高HP帯: 0番セット
					comboChoice = 0;
				}
				else if (hpRatio >= HP_LOW) {
					// 中HP帯: 1番セット
					comboChoice = 1;
				}
				else {
					// 低HP帯: 2番セット（強力）
					comboChoice = 2;
				}
				// ランダム性を少し追加してバリエーションを出す
				if (rand() % 100 < 20) {
					// 20%で別セットを使う（安全策）
					comboChoice = (comboChoice + 1) % static_cast<int>(comboSets_.size());
				}
				activeComboIndex_ = comboChoice;
				comboStep_ = 0;
				comboExecuting_ = true;
				comboActionInProgress_ = false; // 次ループで最初のアクションを開始する
			}
			else {
				// 既にコンボ実行中なら念のため IDLE を維持
			}
		}
		break;
	case ActorState::ATTACK_RANGE:
		if (!attackRegistered_) {
			VECTOR playerPos = player_->GetPos();

			// 発射開始位置（Yだけ上げる）
			VECTOR launchPos = pos_;
			launchPos.y += 00.0f;

			// launchPos から playerPos への方向ベクトル
			VECTOR toPlayer = {
				playerPos.x - launchPos.x,
				playerPos.y - launchPos.y,
				playerPos.z - launchPos.z
			};
			float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
			VECTOR dir = { 0, 0, 0 };
			if (len > 0.0f) {
				dir.x = toPlayer.x / len;
				dir.y = toPlayer.y / len;
				dir.z = toPlayer.z / len;
			}
			VECTOR velocity = { dir.x * 20.0f, dir.y * 20.0f, dir.z * 20.0f };
			auto attack = new RangedAttack(
				-1,         // targetGridIdx
				false,      // isPlayer
				velocity,   // 速度ベクトル
				4.0f,       // lifeTime
				23,         // damage
				this        // shooter
			);
			attack->SetPos(launchPos); // 発射開始位置を明示的に設定
			attackManager_->Add(attack);
			attackRegistered_ = true;
			isAttacking_ = true;
			stateTimer_ = 1.0f;
		}
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
			isAttacking_ = false;
		}
		break;



	case ActorState::ATTACK_NEAR://仮で落雷
		if (!attackRegistered_) {
			int thunderCount = 3 + rand() % 3; // 3～5個
			float thunderHeight = 50.0f;

			// グリッド交点リストを作成 -800, -400, 0, 400, 800,
			std::vector<VECTOR> gridPoints;
			for (int x = -800; x <= 800; x += 400) {
				for (int z = -800; z <= 800; z += 400) {
					VECTOR p = { (float)x, 0.0f, (float)z };
					gridPoints.push_back(p);
				}
			}

			for (int i = 0; i < thunderCount; ++i) {
				int idx = rand() % gridPoints.size();
				VECTOR thunderPos = gridPoints[idx];
				thunderPos.y += thunderHeight; // 上空から落とす

				VECTOR velocity = { 0.0f, -100.0f, 0.0f };

				// グリッド番号を取得
				int gridIdx = AttackBase::CalcGridIndex(thunderPos, false);

				auto thunder = new ThunderAttack(
					gridIdx,
					false,
					velocity,
					1.0f,
					20,
					this
				);
				thunder->SetPos(thunderPos);
				attackManager_->Add(thunder);
			}
			attackRegistered_ = true;
			stateTimer_ = 1.0f;
			isAttacking_ = true;
		}
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
			isAttacking_ = false;
		}
		break;

	case ActorState::ULTIMATE:
		if (!attackRegistered_) {
			// プレイヤーが登録した必殺技コマンドからランダム選択
			if (!attackManager_->registeredCommands_.empty()) {
				int idx = rand() % attackManager_->registeredCommands_.size();
				const auto& pair = attackManager_->registeredCommands_[idx];
				std::string commandStr = pair.first;
				std::string commandId = pair.second;
				StartTypingUltimate(commandStr);
				// UIに敵の詠唱を通知
				UIManager::GetInstance().SetEnemyCasting(commandStr);
			}
			attackRegistered_ = true;
			isAttacking_ = true;
		}
		// タイピング演出が終わったらIDLEに戻す
		if (attackRegistered_ && typingCommand_.empty()) {
			ChangeState(ActorState::IDLE);
			isAttacking_ = false;
			// コンボ実行中ならULTアクションとして扱い、終了させる
			if (comboExecuting_) {
				comboActionInProgress_ = false;
				comboStep_++;
			}
		}
		break;

	case ActorState::STUN:
		stateTimer_ -= 1.0f / 60.0f;
		if (stateTimer_ <= 0.0f) {
			ChangeState(ActorState::IDLE);
		}
		break;
	}

	if (!typingCommand_.empty()) {
		UpdateTypingUltimate(1.0f / 60.0f); // deltaTimeはフレーム時間
	}

	// モデルの位置を反映
	MV1SetPosition(modelId_, pos_);
	// 歩くアニメーション再生
// 攻撃状態の管理（例: isAttacking_ フラグを用意）
	if (isAttacking_) {
		// 攻撃アニメーション開始時のみ再生
		if (animationController_->GetPlayType() != static_cast<int>(ANIM_TYPE::SHOT)) {
			animationController_->Play(static_cast<int>(ANIM_TYPE::SHOT), false);
		}
		// アニメーションが終了したら攻撃状態解除
		if (animationController_->IsEnd()) {
			isAttacking_ = false;
		}
	}
	else {
		// 通常アニメーション
		animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
	}
}

void Enemy::Draw(void)
{
	//if (isNoticeView_)
	//{
	//	// 視野範囲内：ディフューズカラーを赤色にする
	//	MV1SetMaterialDifColor(modelId_, 0, GetColorF(1.0f, 0.0f, 0.0f, 1.0f));
	//}
	//else if (isNoticeHearing_)
	//{
	//	// 聴覚範囲内：ディフューズカラーを黄色にする
	//	MV1SetMaterialDifColor(modelId_, 0, GetColorF(1.0f, 1.0f, 0.0f, 1.0f));
	//}
	//else
	//{
	//	// 視野範囲外：ディフューズカラーを灰色にする
	//	MV1SetMaterialDifColor(modelId_, 0, GetColorF(0.5f, 0.5f, 0.5f, 1.0f));
	//}

	ActorBase::Draw();

	// 視野描画
	//DrawViewRange();
	// --- カプセル当たり判定のデバッグ表示 ---
	VECTOR start = VAdd(pos_, startCapsulePos_);
	VECTOR end = VAdd(pos_, endCapsulePos_);
	float radius = capsuleRadius_;
	//DrawCapsule3D(start, end, radius, 16, GetColor(0, 255, 0), false,false);

	//DrawFormatString(700, 10, 0xFFFFFF, "HP: %d / %d", GetHp(), GetMaxHp());

	// タイピング進捗のデバッグ表示（追加）
	if (!typingCommand_.empty()) {
		// 合計文字数（UTF-8文字数）
		size_t totalChars = GetUtf8CharCount(typingCommand_);
		// 現在の入力済み文字数（経過時間比率から算出）
		float ratio = 0.0f;
		if (typingWait_ > 0.0f) {
			ratio = typingElapsed_ / typingWait_;
			if (ratio < 0.0f) ratio = 0.0f;
			if (ratio > 1.0f) ratio = 1.0f;
		}
		size_t typedChars = static_cast<size_t>(ratio * static_cast<float>(totalChars));
		if (typedChars > totalChars) typedChars = totalChars;

		// 先頭 typedChars 文字を切り出す
		std::string typedStr = Utf8Substring(typingCommand_, typedChars);

		// フルコマンドを表示（オレンジ）
		DrawFormatString(700, 50, 0xFFAA00, "UltimateCmd: %s", typingCommand_.c_str());
		// 入力済み部分を別色で表示（緑）
		DrawFormatString(700, 70, 0x00FF00, "Typed: %s (%zu/%zu)", typedStr.c_str(), typedChars, totalChars);
	}

	//// --- 攻撃ステートのデバッグ表示 ---
	//const char* stateStr = "";
	//switch (state_) {
	//case ActorState::IDLE:         stateStr = "IDLE"; break;
	//case ActorState::ATTACK_NEAR:  stateStr = "ATTACK_NEAR"; break;
	//case ActorState::ATTACK_RANGE: stateStr = "ATTACK_RANGE"; break;
	//case ActorState::ULTIMATE:     stateStr = "ULTIMATE"; break;
	//case ActorState::STUN:         stateStr = "STUN"; break;
	//default:                       stateStr = "UNKNOWN"; break;
	//}
	//DrawFormatString(700, 30, 0x00FF00, "EnemyState: %s", stateStr);

	//// --- デバッグ用：ULTIMATEコマンド名の表示 ---
	//if (!typingCommand_.empty()) {
	//	DrawFormatString(700, 50, 0xFFAA00, "UltimateCmd: %s", typingCommand_.c_str());
	//}

	ActorBase::Draw();
}

// （以下の関数群は変更なし）
void Enemy::InitLoad(void)
{
	// モデルの読み込み
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Player/Player.mv1").c_str());
}

void Enemy::InitTransform(void)
{
	// モデルの角度
	angle_ = { 0.0f, AsoUtility::Deg2RadF(90), 0.0f };
	localAngle_ = { 0.0f, AsoUtility::Deg2RadF(270.0f), 0.0f };

	moveDir_ = { sinf(angle_.y), 0.0f, cosf(angle_.y) };
	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);
	MV1SetRotationMatrix(modelId_, mat);

	// グリッド外に配置（例: X=-500, Y=-400, Z=1000）
	//pos_ = { 0.0f, -2000.0f, 4000.0f };
	pos_ = { 0.0f, -0, 1200 };

	// ボスサイズに拡大（例: X=30, Y=18, Z=18）
	//scale_ = { 30, 18, 18 };
	scale_ = { 1, 1, 1 };
	MV1SetPosition(modelId_, pos_);
	MV1SetScale(modelId_, scale_);

	// 当たり判定も大きく
	//startCapsulePos_ = { 0.0f, 2500, 0.0f };
	//endCapsulePos_ = { 0.0f, 90.0f, 0.0f };
	//capsuleRadius_ = 400.0f;

	startCapsulePos_ = { 0.0f,110,0.0f };
	endCapsulePos_ = { 0.0f,30.0f,0.0f };
	capsuleRadius_ = 20.0f;

	isCollision_ = true;
	moveCenterX_ = pos_.x;
}


void Enemy::InitAnimation(void)
{
	// モデルアニメーション制御の初期化
	animationController_ = new AnimationController(modelId_);

	// アニメーションの追加
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::IDLE), 0.5f, Application::PATH_MODEL + "Player/Idle.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::WALK), 0.5f, Application::PATH_MODEL + "Player/Walk.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::SHOT), 0.f, Application::PATH_MODEL + "Player/Shot.mv1");
	// 初期アニメーションの再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
}

void Enemy::InitPost(void)
{
	isNoticeView_ = false;
	isNoticeHearing_ = false;
}

//void Enemy::DrawViewRange(void)
// (省略: コメントアウトそのまま)

void Enemy::ApplyDamage(int damage) {
	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
	// ボス用の追加処理（例：エフェクト、SE、ひるみゲージ加算など）
}

void Enemy::AddStun(int value) {
	stunGauge_ += value;
	if (stunGauge_ > maxStunGauge_) stunGauge_ = maxStunGauge_;
	// ボス用の追加処理
}

void Enemy::OnStunned() {
	// ボスがひるんだ時の処理
	// 例：ステートをSTUNに変更、エフェクト再生など
}

bool Enemy::IsDead() const {
	return hp_ <= 0;
}


void Enemy::ChangeState(ActorState state)
{
	state_ = state;
	attackRegistered_ = false;
	switch (state_) {
	case ActorState::IDLE:
		stateTimer_ = 3.0f; // 待機時間例
		break;
	case ActorState::ATTACK_NEAR:
		// 近接攻撃準備
		break;
	case ActorState::ATTACK_RANGE:

		break;
	case ActorState::ULTIMATE:
		// 必殺技準備
		break;
	case ActorState::STUN:
		stateTimer_ = 2.0f; // ひるみ時間例
		break;
	}
}

ActorBase::ActorState Enemy::GetState() const
{
	return ActorState();
}

void Enemy::StartTypingUltimate(const std::string& command) {
	typingCommand_ = command;
	typingElapsed_ = 0.0f;

	// UIに詠唱を通知
	UIManager::GetInstance().SetEnemyCasting(command);

	// ★ ここが修正のキモ！ バイト数ではなく実際の文字数を数える
	size_t charCount = GetUtf8CharCount(command);

	// 1文字0.2秒 × 実際の文字数
	typingWait_ = static_cast<float>(charCount) * 0.2f;
}

void Enemy::UpdateTypingUltimate(float deltaTime) {
	if (!typingCommand_.empty()) {
		typingElapsed_ += deltaTime;
		if (typingElapsed_ >= typingWait_) {
			// 必殺技発動
			// コマンドID取得
			auto it = std::find_if(
				attackManager_->registeredCommands_.begin(),
				attackManager_->registeredCommands_.end(),
				[&](const auto& pair) { return pair.first == typingCommand_; }
			);
			if (it != attackManager_->registeredCommands_.end()) {
				std::string commandId = it->second;
				auto dataIt = attackManager_->ultimateCommandDataMap_.find(commandId);
				if (dataIt != attackManager_->ultimateCommandDataMap_.end()) {
					const auto& data = dataIt->second;

					// 例：プレイヤー方向に直進する必殺技
					VECTOR playerPos = player_->GetPos();
					VECTOR toPlayer = {
						playerPos.x - pos_.x,
						playerPos.y - pos_.y,
						playerPos.z - pos_.z
					};
					float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
					VECTOR dir = { 0, 0, 0 };
					if (len > 0.0f) {
						dir.x = toPlayer.x / len;
						dir.y = toPlayer.y / len;
						dir.z = toPlayer.z / len;
					}
					VECTOR velocity = { dir.x * data.speed, dir.y * data.speed, dir.z * data.speed };

					auto ultimate = new UltimateAttack(
						-1,         // targetGridIdx
						false,      // isPlayer
						velocity,   // 速度
						1.0f,       // lifeTime
						data.damage,// damage
						this        // shooter
					);
					ultimate->SetPos(pos_); // 発射位置を明示的に設定
					attackManager_->Add(ultimate);
				}
			}
			typingCommand_.clear();
			// UIに詠唱終了を通知（空にする）
			UIManager::GetInstance().SetEnemyCasting("");
		}
	}
}