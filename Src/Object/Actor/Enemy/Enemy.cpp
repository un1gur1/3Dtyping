#include "Enemy.h"
#include <random>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <unordered_map>
#include <sstream>

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
#include "../../../Common/RomanjiConverter.h"

// =======================================================
// ユーティリティ関数群（無名名前空間）
// =======================================================
namespace {
	static size_t GetUtf8CharCount(const std::string& s) {
		size_t count = 0;
		for (unsigned char c : s) {
			if ((c & 0xC0) != 0x80) count++;
		}
		return count;
	}

	static bool IsSpaceSafe(const unsigned char ch) {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}

	static std::string ToLowerTrim(const std::string& s) {
		std::string t = s;
		t.erase(t.begin(), std::find_if(t.begin(), t.end(), [](unsigned char ch) { return !IsSpaceSafe(ch); }));
		t.erase(std::find_if(t.rbegin(), t.rend(), [](unsigned char ch) { return !IsSpaceSafe(ch); }).base(), t.end());
		for (char& c : t) {
			const unsigned char uc = static_cast<unsigned char>(c);
			if (uc < 128) c = static_cast<char>(std::tolower(uc));
		}
		return t;
	}

	static bool IsLikelyRomanji(const std::string& s) {
		if (s.empty()) return false;
		bool hasAlpha = false;
		for (unsigned char c : s) {
			if (c >= 128) return false;
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) hasAlpha = true;
		}
		return hasAlpha;
	}

	static std::string ConvertRomanjiToHiragana(const std::string& in) {
		RomanjiConverter conv;
		std::string filtered;
		for (unsigned char c : in) {
			if (c != ' ') filtered += static_cast<char>(std::tolower(c));
		}
		return conv.convert(filtered);
	}

	static std::string ConvertIfRomanji(const std::string& s) {
		if (IsLikelyRomanji(s)) {
			const std::string hira = ConvertRomanjiToHiragana(s);
			if (!hira.empty()) return hira;
		}
		return s;
	}

	static const std::unordered_map<std::string, std::string>& GetWordTypeMap() {
		static std::unordered_map<std::string, std::string> map;
		if (!map.empty()) return map;

		std::ifstream ifs("Data/CSV/Word.csv");
		if (!ifs.is_open()) return map;

		std::string line;
		while (std::getline(ifs, line)) {
			if (line.empty()) continue;
			std::istringstream iss(line);
			std::string name, type;
			if (!std::getline(iss, name, ',') || !std::getline(iss, type, ',')) continue;

			const std::string norm = ConvertIfRomanji(ToLowerTrim(name));
			const std::string ttype = ToLowerTrim(type);
			if (!norm.empty() && !ttype.empty()) {
				map.emplace(norm, ttype);
			}
		}
		return map;
	}

	static std::string PickAttackCommandFromNormalList(const std::unordered_map<std::string, std::string>& wordMap) {
		const auto& normalCmds = UIManager::GetInstance().normalCommandList_;
		std::vector<std::string> candidates;
		for (const auto& orig : normalCmds) {
			const std::string norm = ConvertIfRomanji(ToLowerTrim(orig));
			const auto it = wordMap.find(norm);
			if (it != wordMap.end()) {
				const std::string& typeLower = it->second;
				if (typeLower == "attack" || typeLower == "shoot") {
					candidates.push_back(norm);
				}
			}
		}
		if (candidates.empty()) return std::string();
		return candidates[rand() % candidates.size()];
	}

	static std::vector<std::vector<Enemy::EnemyAction>> CreateDefaultCombos() {
		using A = Enemy::EnemyAction;
		std::vector<std::vector<A>> sets;
		sets.push_back({ A::ATTACK_RANGE, A::ATTACK_RANGE, A::ATTACK_RANGE });
		sets.push_back({ A::ATTACK_RANGE, A::THUNDER, A::ATTACK_RANGE });
		sets.push_back({ A::THUNDER, A::ATTACK_NEAR, A::THUNDER, A::ULTIMATE });
		return sets;
	}
}

// =======================================================
// 初期化・破棄
// =======================================================
Enemy::Enemy(Player* player) : player_(player) {
	comboSets_ = CreateDefaultCombos();
}

Enemy::~Enemy(void) {}

// =======================================================
// Update
// =======================================================
void Enemy::Update(void) {
	ActorBase::Update();
	const float frameDt = 1.0f / 60.0f;

	// アニメーション制御
	if (isAttacking_) {
		if (animationController_->GetPlayType() != static_cast<int>(ANIM_TYPE::SHOT)) {
			animationController_->Play(static_cast<int>(ANIM_TYPE::SHOT), false);
		}
		if (animationController_->IsEnd()) isAttacking_ = false;
	}
	else {
		animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
	}
	animationController_->Update();

	// フワフワ移動
	if (!isAttacking_ && typingCommand_.empty()) {
		moveTime_ += frameDt;
		pos_.x = 30.0f * sinf(moveTime_ * DX_TWO_PI / 2.0f);
	}
	MV1SetPosition(modelId_, pos_);

	// --- 1. クールダウン・待機処理 ---
	if (attackCooldownTimer_ > 0.0f) {
		attackCooldownTimer_ -= frameDt;
		if (attackCooldownTimer_ < 0.0f) attackCooldownTimer_ = 0.0f;
	}

	if (waitingForAttackFinish_) {
		if (attackCooldownTimer_ <= 0.0f && !IsMyAttackAlive()) {
			waitingForAttackFinish_ = false;
			StartNextComboStep();
		}
		// UIManagerに待機状態を送信
		UIManager::GetInstance().SetEnemyWaitState(waitingForAttackFinish_, attackCooldownTimer_);
		return;
	}

	// --- 2. 詠唱処理 ---
	if (!typingCommand_.empty()) {
		UpdateTypingUltimate(frameDt);
		return;
	}

	// 待機でも詠唱でもない場合はUIManagerの通知をクリア
	UIManager::GetInstance().SetEnemyWaitState(false, 0.0f);

	// --- 3. コンボ開始 ---
	if (state_ == ActorState::IDLE) {
		stateTimer_ -= frameDt;
		if (stateTimer_ <= 0.0f) {

			const float hpRatio = (maxHp_ > 0) ? static_cast<float>(hp_) / static_cast<float>(maxHp_) : 1.0f;
			if (hpRatio < 0.5f && rand() % 100 < 30) {
				activeComboIndex_ = -1;
				StartTypingUltimate("ほろびのばーすとすとりーむ");
				PreparePlannedAttackData();
				return;
			}

			if (hpRatio >= HP_HIGH) activeComboIndex_ = 0;
			else if (hpRatio >= HP_LOW) activeComboIndex_ = 1;
			else activeComboIndex_ = 2;

			if (rand() % 100 < 20) {
				activeComboIndex_ = (activeComboIndex_ + 1) % comboSets_.size();
			}

			comboStep_ = 0;
			StartNextComboStep();
		}
	}
}

// =======================================================
// コンボ制御
// =======================================================
void Enemy::StartNextComboStep() {
	if (activeComboIndex_ < 0 || activeComboIndex_ >= static_cast<int>(comboSets_.size())) {
		ChangeState(ActorState::IDLE);
		stateTimer_ = 2.0f;
		return;
	}

	if (comboStep_ >= static_cast<int>(comboSets_[activeComboIndex_].size())) {
		activeComboIndex_ = -1;
		ChangeState(ActorState::IDLE);
		stateTimer_ = 2.0f;
		return;
	}

	const auto& wordMap = GetWordTypeMap();
	const EnemyAction nextAction = comboSets_[activeComboIndex_][comboStep_];
	std::string cmd;

	if (nextAction == EnemyAction::ULTIMATE) {
		if (attackManager_ && !attackManager_->registeredCommands_.empty()) {
			cmd = attackManager_->registeredCommands_[rand() % attackManager_->registeredCommands_.size()].first;
		}
		ChangeState(ActorState::ULTIMATE);
	}
	else {
		cmd = PickAttackCommandFromNormalList(wordMap);
		if (cmd.empty() && attackManager_ && !attackManager_->registeredCommands_.empty()) {
			cmd = attackManager_->registeredCommands_[rand() % attackManager_->registeredCommands_.size()].first;
		}
		ChangeState((nextAction == EnemyAction::ATTACK_RANGE) ? ActorState::ATTACK_RANGE : ActorState::ATTACK_NEAR);
	}

	if (!cmd.empty()) {
		StartTypingUltimate(cmd);
		PreparePlannedAttackData();
		isAttacking_ = true;
	}
	else {
		comboStep_ = 999;
		StartNextComboStep();
	}
}

bool Enemy::IsMyAttackAlive() const {
	if (!attackManager_) return false;
	for (const auto* a : attackManager_->GetAttacks()) {
		if (a && a->GetShooter() == this && a->IsAlive()) {
			return true;
		}
	}
	return false;
}

// =======================================================
// 詠唱・攻撃データ
// =======================================================
void Enemy::StartTypingUltimate(const std::string& command) {
	typingCommand_ = command;
	typingElapsed_ = 0.0f;

	const size_t charCount = GetUtf8CharCount(command);
	const float hpRatio = (maxHp_ > 0) ? static_cast<float>(hp_) / static_cast<float>(maxHp_) : 1.0f;
	const float perChar = (hpRatio >= HP_HIGH) ? 1.0f : 0.2f;

	typingWait_ = static_cast<float>(charCount) * perChar;

	// UIに詠唱開始を通知
	UIManager::GetInstance().SetEnemyCasting(typingCommand_, typingElapsed_, typingWait_);
}

void Enemy::UpdateTypingUltimate(const float deltaTime) {
	if (typingCommand_.empty()) return;

	typingElapsed_ += deltaTime;

	// 毎フレームUIに進捗を通知
	UIManager::GetInstance().SetEnemyCasting(typingCommand_, typingElapsed_, typingWait_);

	if (typingElapsed_ < typingWait_) return;

	// --- 詠唱完了発射 ---
	if (!plannedAttacks_.empty() && attackManager_) {
		for (const auto& pa : plannedAttacks_) {
			if (pa.kind == PlannedAttack::Kind::ULTIMATE) {
				auto ultimate = new UltimateAttack(-1, false, pa.velocity, 1.0f, pa.damage, this);
				ultimate->SetPos(pa.pos);
				attackManager_->Add(ultimate);
			}
			else if (pa.kind == PlannedAttack::Kind::THUNDER) {
				for (const auto& tpos : pa.thunderPositions) {
					const VECTOR velocity = { 0.0f, -100.0f, 0.0f };
					const int gridIdx = AttackBase::CalcGridIndex(tpos, false);
					auto thunder = new ThunderAttack(gridIdx, false, velocity, 1.0f, pa.damage, this);
					thunder->SetPos(tpos);
					attackManager_->Add(thunder);
				}
			}
			else if (pa.kind == PlannedAttack::Kind::RANGED) {
				auto ranged = new RangedAttack(-1, false, pa.velocity, 4.0f, pa.damage, this);
				ranged->SetPos(pa.pos);
				attackManager_->Add(ranged);
			}
		}
	}

	plannedAttacks_.clear();
	typingCommand_.clear();

	// 詠唱終了をUIに通知
	UIManager::GetInstance().SetEnemyCasting("", 0.0f, 0.0f);

	isAttacking_ = false;
	ChangeState(ActorState::IDLE);

	attackCooldownTimer_ = attackCooldown_;
	waitingForAttackFinish_ = true;
	comboStep_++;
}

void Enemy::PreparePlannedAttackData() {
	plannedAttacks_.clear();
	if (typingCommand_.empty()) return;

	const auto makeFallbackData = [&](const std::string& key) {
		const std::size_t hv = std::hash<std::string>{}(key);
		struct { int damage; float speed; } d;
		d.damage = 15 + static_cast<int>(hv % 36);
		d.speed = 10.0f + static_cast<float>((hv / 37) % 21);
		return d;
		};

	int damage = 0;
	float speed = 0.0f;
	std::string typeLower = "";
	const auto& wordMap = GetWordTypeMap();

	if (attackManager_) {
		const auto it = std::find_if(attackManager_->registeredCommands_.begin(), attackManager_->registeredCommands_.end(),
			[&](const auto& pair) { return pair.first == typingCommand_; });

		if (it != attackManager_->registeredCommands_.end()) {
			const std::string commandId = it->second;
			const auto dataIt = attackManager_->ultimateCommandDataMap_.find(commandId);
			if (dataIt != attackManager_->ultimateCommandDataMap_.end()) {
				damage = dataIt->second.damage;
				speed = dataIt->second.speed;
			}
		}
	}

	if (damage == 0) {
		const auto fb = makeFallbackData(typingCommand_);
		damage = fb.damage;
		speed = fb.speed;
	}

	const auto wit = wordMap.find(typingCommand_);
	if (wit != wordMap.end()) {
		typeLower = wit->second;
	}

	if (state_ == ActorState::ULTIMATE) {
		const VECTOR playerPos = player_->GetPos();
		const VECTOR toPlayer = { playerPos.x - pos_.x, playerPos.y - pos_.y, playerPos.z - pos_.z };
		const float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
		VECTOR dir = { 0,0,0 };
		if (len > 0.0f) { dir.x = toPlayer.x / len; dir.y = toPlayer.y / len; dir.z = toPlayer.z / len; }

		PlannedAttack pa;
		pa.kind = PlannedAttack::Kind::ULTIMATE;
		pa.pos = pos_;
		pa.velocity = { dir.x * speed, dir.y * speed, dir.z * speed };
		pa.damage = damage;
		plannedAttacks_.push_back(pa);
	}
	else if (typeLower == "shoot") {
		const int thunderCount = 3 + rand() % 3;
		std::vector<VECTOR> positions;

		// 1発目は詠唱開始時のプレイヤー足元へ
		VECTOR targetPos = player_->GetPos();
		targetPos.y = 50.0f;
		positions.push_back(targetPos);

		// 残りはランダム
		for (int i = 1; i < thunderCount; ++i) {
			const int gx = static_cast<int>(targetPos.x) + (-400 + (rand() % 3) * 400);
			const int gz = static_cast<int>(targetPos.z) + (-400 + (rand() % 3) * 400);
			positions.push_back({ static_cast<float>(gx), 50.0f, static_cast<float>(gz) });
		}

		PlannedAttack pa;
		pa.kind = PlannedAttack::Kind::THUNDER;
		pa.damage = damage;
		pa.thunderPositions = std::move(positions);
		plannedAttacks_.push_back(pa);
	}
	else {
		const VECTOR playerPos = player_->GetPos();
		const VECTOR toPlayer = { playerPos.x - pos_.x, playerPos.y - pos_.y, playerPos.z - pos_.z };
		const float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
		VECTOR dir = { 0,0,0 };
		if (len > 0.0f) { dir.x = toPlayer.x / len; dir.y = toPlayer.y / len; dir.z = toPlayer.z / len; }

		PlannedAttack pa;
		pa.kind = PlannedAttack::Kind::RANGED;
		pa.pos = pos_;
		pa.velocity = { dir.x * speed, dir.y * speed, dir.z * speed };
		pa.damage = damage;
		plannedAttacks_.push_back(pa);
	}
}

void Enemy::OnAttackCancelled() {
	typingCommand_.clear();
	UIManager::GetInstance().SetEnemyCasting("", 0.0f, 0.0f);
	plannedAttacks_.clear();
	isAttacking_ = false;
	waitingForAttackFinish_ = false;
	ChangeState(ActorState::IDLE);
}

// =======================================================
// その他システム
// =======================================================
void Enemy::Draw(void) {
	ActorBase::Draw();

	// 詠唱中の攻撃位置（3D空間の予測線や球）のみ描画
	if (!typingCommand_.empty() && !plannedAttacks_.empty()) {
		for (const auto& pa : plannedAttacks_) {
			if (pa.kind == PlannedAttack::Kind::THUNDER) {
				for (const auto& tpos : pa.thunderPositions) {
					VECTOR drawPos = tpos;
					drawPos.y += 5.0f;
					DrawSphere3D(drawPos, 30.0f, 12, GetColor(255, 160, 0), true, false);
				}
			}
			else if (pa.kind == PlannedAttack::Kind::RANGED || pa.kind == PlannedAttack::Kind::ULTIMATE) {
				const VECTOR startPos = pa.pos;
				const float lifetime = (pa.kind == PlannedAttack::Kind::ULTIMATE) ? 1.0f : 4.0f;
				const VECTOR impact = { startPos.x + pa.velocity.x * lifetime,
								  startPos.y + pa.velocity.y * lifetime,
								  startPos.z + pa.velocity.z * lifetime };
				const unsigned int lineColor = (pa.kind == PlannedAttack::Kind::ULTIMATE) ? GetColor(255, 80, 80) : GetColor(120, 200, 255);

				DrawLine3D(startPos, impact, lineColor);
				VECTOR ip = impact;
				ip.y += 5.0f;
				DrawSphere3D(ip, 18.0f, 12, lineColor, true, false);
			}
		}
	}
}

void Enemy::ChangeState(const ActorState state) {
	state_ = state;
	switch (state_) {
	case ActorState::IDLE:
		break;
	case ActorState::STUN:
		stateTimer_ = 2.0f;
		break;
	default:
		break;
	}
}

ActorBase::ActorState Enemy::GetState() const { return state_; }

void Enemy::InitLoad(void) { modelId_ = MV1LoadModel((Application::PATH_MODEL + "Player/Player.mv1").c_str()); }
void Enemy::InitTransform(void) {
	angle_ = { 0.0f, AsoUtility::Deg2RadF(90), 0.0f };
	localAngle_ = { 0.0f, AsoUtility::Deg2RadF(270.0f), 0.0f };
	moveDir_ = { sinf(angle_.y), 0.0f, cosf(angle_.y) };
	MV1SetRotationMatrix(modelId_, MatrixUtility::Multiplication(localAngle_, angle_));
	pos_ = { 0.0f, 0.0f, 1200.0f };
	scale_ = { 1.0f, 1.0f, 1.0f };
	MV1SetPosition(modelId_, pos_);
	MV1SetScale(modelId_, scale_);
	startCapsulePos_ = { 0.0f, 110.0f, 0.0f };
	endCapsulePos_ = { 0.0f, 30.0f, 0.0f };
	capsuleRadius_ = 20.0f;
	isCollision_ = true;
	moveCenterX_ = pos_.x;
}
void Enemy::InitAnimation(void) {
	animationController_ = new AnimationController(modelId_);
	animationController_->Add(static_cast<int>(ANIM_TYPE::IDLE), 0.5f, Application::PATH_MODEL + "Player/Idle.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::WALK), 0.5f, Application::PATH_MODEL + "Player/Walk.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::SHOT), 0.0f, Application::PATH_MODEL + "Player/Shot.mv1");
	animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
}
void Enemy::InitPost(void) {}
void Enemy::ApplyDamage(const int damage) {
	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
}
void Enemy::AddStun(const int value) {
	stunGauge_ += value;
	if (stunGauge_ > maxStunGauge_) stunGauge_ = maxStunGauge_;
}
void Enemy::OnStunned() {}
bool Enemy::IsDead() const { return hp_ <= 0; }