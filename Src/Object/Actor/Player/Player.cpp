#include "Player.h"

#include <DxLib.h>
#include <random> 
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cctype>

#include "../../../Common/RomanjiConverter.h"
#include "../../../Application.h"
#include "../../../Input/InputManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../Common/AnimationController.h"
#include "../../Attack/AttackManager.h"
#include "../../Attack/RangedAttack/RangedAttack.h"
#include "../../Attack/Magic/ThunderAttack.h"
#include "../../Attack/UltimateAttack/UltimateAttack.h"
#include "../../../Camera/Camera.h"
#include "../../../Common/UiManager.h"

// 追加インクルード：新しい攻撃クラス
#include "../../Attack/DarkAttack/DarkAttack.h"
#include "../../Attack/HealAttack/HealAttack.h"
#include "../../Attack/MeteorAttack/MeteorAttack.h"
#include "../../Attack/SwordAttack/SwordAttack.h"

// =======================================================
// 正規化ヘルパー（無名名前空間）
// =======================================================
namespace {
	static bool IsLikelyRomanji(const std::string& s) {
		if (s.empty()) return false;
		bool hasAlpha = false;
		for (unsigned char c : s) {
			if (c >= 128) return false;
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				hasAlpha = true;
			}
		}
		return hasAlpha;
	}

	static std::string ConvertRomanjiToHiragana(const std::string& in) {
		RomanjiConverter conv;
		std::string filtered;
		for (unsigned char c : in) {
			if (c != ' ') {
				filtered += static_cast<char>(std::tolower(c));
			}
		}
		return conv.convert(filtered);
	}

	static std::string ConvertIfRomanji(const std::string& s) {
		if (IsLikelyRomanji(s)) {
			std::string hira = ConvertRomanjiToHiragana(s);
			if (!hira.empty()) return hira;
		}
		return s;
	}

	static bool IsSpaceSafe(unsigned char ch) {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}

	static std::string ToLowerTrim(const std::string& s) {
		std::string t = s;
		t.erase(t.begin(), std::find_if(t.begin(), t.end(), [](unsigned char ch) {
			return !IsSpaceSafe(ch);
			}));
		t.erase(std::find_if(t.rbegin(), t.rend(), [](unsigned char ch) {
			return !IsSpaceSafe(ch);
			}).base(), t.end());
		for (char& c : t) {
			unsigned char uc = static_cast<unsigned char>(c);
			if (uc < 128) c = static_cast<char>(std::tolower(uc));
		}
		return t;
	}
}
// =======================================================

Player::Player(Camera* camera) : camera_(camera)
{
}

Player::~Player(void)
{
}

Player::CommandType Player::StringToCommandType(const std::string& str)
{
	if (str == "MOVE_UP") return CommandType::MOVE_UP;
	if (str == "MOVE_DOWN") return CommandType::MOVE_DOWN;
	if (str == "MOVE_LEFT") return CommandType::MOVE_LEFT;
	if (str == "MOVE_RIGHT") return CommandType::MOVE_RIGHT;
	if (str == "MOVE_UP_RIGHT") return CommandType::MOVE_UP_RIGHT;
	if (str == "MOVE_UP_LEFT") return CommandType::MOVE_UP_LEFT;
	if (str == "MOVE_DOWN_RIGHT") return CommandType::MOVE_DOWN_RIGHT;
	if (str == "MOVE_DOWN_LEFT") return CommandType::MOVE_DOWN_LEFT;
	if (str == "DODGE") return CommandType::DODGE;
	if (str == "MOVE_RANDOM") return CommandType::MOVE_RANDOM;
	if (str == "SHOOT") return CommandType::SHOOT;
	if (str == "ATTACK") return CommandType::ATTACK;
	return CommandType::UNKNOWN;
}

void Player::InitLoad(void)
{
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Player/Player.mv1").c_str());
	LoadMoveWordDict("Data/CSV/Word.csv");

	keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
	SetActiveKeyInput(keyInputHandle_);
	isInputActive_ = true;
	inputBuf_[0] = '\0';
}

void Player::InitTransform(void)
{
	logicPos_ = { 0.0f, 0.0f, 0.0f };
	drawPos_ = logicPos_;
	gridPos_ = { 0, 0 };
	targetGridPos_ = gridPos_;

	angle_ = { 0.0f, 0.0f, 0.0f };
	localAngle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f };

	moveDir_ = { sinf(angle_.y), 0.0f, cosf(angle_.y) };
	preInputDir_ = moveDir_;

	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);
	MV1SetRotationMatrix(modelId_, mat);

	pos_ = AsoUtility::VECTOR_ZERO;
	MV1SetPosition(modelId_, pos_);

	startCapsulePos_ = { 0.0f, 110.0f, 0.0f };
	endCapsulePos_ = { 0.0f, 30.0f, 0.0f };
	capsuleRadius_ = 20.0f;
	isCollision_ = true;
}

void Player::InitAnimation(void)
{
	animationController_ = new AnimationController(modelId_);

	animationController_->Add(static_cast<int>(ANIM_TYPE::IDLE), 0.5f, Application::PATH_MODEL + "Player/Idle.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::WALK), 0.4f, Application::PATH_MODEL + "Player/FastRun.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK2), 0.4f, Application::PATH_MODEL + "Player/Attack2.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK3), 0.4f, Application::PATH_MODEL + "Player/Attack3.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::BOXING), 0.4f, Application::PATH_MODEL + "Player/Boxing.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::FALLING), 0.4f, Application::PATH_MODEL + "Player/Falling.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::FLYING), 0.4f, Application::PATH_MODEL + "Player/Flying.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::JUMP), 0.4f, Application::PATH_MODEL + "Player/Jump.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::JUMP_ATTACK), 0.4f, Application::PATH_MODEL + "Player/JumpAttack.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::JUMP_HOVER), 0.4f, Application::PATH_MODEL + "Player/JumpHover.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::JUMPING), 0.4f, Application::PATH_MODEL + "Player/Jumping.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::JUMP_RISING), 0.4f, Application::PATH_MODEL + "Player/JumpRising.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::SHOT), 0.4f, Application::PATH_MODEL + "Player/Shot.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::THROW), 0.4f, Application::PATH_MODEL + "Player/Throw.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::VICTORY), 0.4f, Application::PATH_MODEL + "Player/Victory.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::WALK_LOOP), 0.4f, Application::PATH_MODEL + "Player/Walk.mv1");
	animationController_->Add(static_cast<int>(ANIM_TYPE::WARP_POSE), 0.4f, Application::PATH_MODEL + "Player/WarpPose.mv1");

	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{
	if (camera_) {
		camera_->SetFollow(this);
	}
}

void Player::Update(void)
{
	ActorBase::Update();

	std::string inputStr(inputBuf_);
	std::string hiraText = ConvertIfRomanji(inputStr);

	std::string prevInputStr = inputText_;
	std::string prevHiraText = ConvertIfRomanji(prevInputStr);

	UIManager::GetInstance().SetTypingStrings(inputStr, hiraText, prevInputStr, prevHiraText);

	// --- 必殺技コマンド登録モード ---
	if (!isRegisteringUltimate_) {
		if (CheckHitKey(KEY_INPUT_F1)) {
			isRegisteringUltimate_ = true;
			registerKeyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(registerKeyInputHandle_);
			registerInputBuf_[0] = '\0';
		}
	}
	else {
		GetKeyInputString(registerInputBuf_, registerKeyInputHandle_);

		if (CheckKeyInput(registerKeyInputHandle_) == 1) {
			std::string raw(registerInputBuf_);

			DeleteKeyInput(registerKeyInputHandle_);
			SetActiveKeyInput(-1);
			isRegisteringUltimate_ = false;

			keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandle_);
			isInputActive_ = true;
			inputBuf_[0] = '\0';

			if (attackManager_ && !raw.empty()) {
				std::string commandStr = ConvertIfRomanji(ToLowerTrim(raw));

				std::string commandId = attackManager_->RegisterUltimateCommand(commandStr, 5);
				attackManager_->ReloadCommands();
			}
			return;
		}

		if (CheckHitKey(KEY_INPUT_ESCAPE)) {
			DeleteKeyInput(registerKeyInputHandle_);
			SetActiveKeyInput(-1);
			isRegisteringUltimate_ = false;

			keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandle_);
			isInputActive_ = true;
			inputBuf_[0] = '\0';
		}
		return;
	}

	// グリッド移動処理
	if (isMovingOnGrid_) {
		VECTOR targetLogicPos = {
			targetGridPos_.x * gridSize_,
			0.0f,
			targetGridPos_.z * gridSize_
		};

		VECTOR diff = {
			targetLogicPos.x - logicPos_.x,
			0.0f,
			targetLogicPos.z - logicPos_.z
		};

		if (diff.x != 0.0f || diff.z != 0.0f) {
			angle_.y = atan2f(diff.x, diff.z);
		}

		float speed = 10.0f;
		float dist = sqrtf(diff.x * diff.x + diff.z * diff.z);
		if (dist < speed) {
			logicPos_.x = targetLogicPos.x;
			logicPos_.z = targetLogicPos.z;
			gridPos_ = targetGridPos_;
			isMovingOnGrid_ = false;
		}
		else {
			logicPos_.x += diff.x / dist * speed;
			logicPos_.z += diff.z / dist * speed;
		}
	}

	// アニメーションの更新
	animationController_->Update();

	if (isAttacking_) {
		if (animationController_->GetPlayType() != static_cast<int>(ANIM_TYPE::SHOT)) {
			animationController_->Play(static_cast<int>(ANIM_TYPE::SHOT), false);
		}
		if (animationController_->IsEnd()) {
			isAttacking_ = false;
		}
	}
	else {
		if (isMovingOnGrid_) {
			animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
		}
		else {
			animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
		}
	}

	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);
	MV1SetRotationMatrix(modelId_, mat);

	pos_ = logicPos_;
}

void Player::Draw(void)
{
	ActorBase::Draw();
}

void Player::Release(void)
{
	ActorBase::Release();
}

// =======================================================
// CSV読み込み（ゲーム開始時に1度だけ読み、辞書に記憶）
// =======================================================
void Player::LoadMoveWordDict(const std::string& path)
{
	std::ifstream file(path);
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string word, typeStr;
		if (std::getline(iss, word, ',') && std::getline(iss, typeStr, ',')) {
			std::string lower = ToLowerTrim(word);
			std::string key = ConvertIfRomanji(lower);
			std::string ttype = ToLowerTrim(typeStr);

			CommandType type = StringToCommandType(typeStr);
			commandMap_[key] = type;
			magicTypeMap_[key] = ttype; // 魔法属性も記憶
		}
	}
}

// =======================================================
// 完全統一：すべての攻撃・魔法を生成する処理
// =======================================================
void Player::ExecuteMagic(const std::string& magicType, int damage, float speed)
{
	VECTOR pos = GetPos();
	VECTOR vel = { 0.0f, 0.0f, speed };
	AttackBase* newAttack = nullptr;

	if (magicType == "dark") {
		newAttack = new DarkAttack(-1, true, vel, 1.0f, damage, this);
		newAttack->SetPos(pos);
	}
	else if (magicType == "heal") {
		newAttack = new HealAttack(-1, true, vel, 1.0f, damage, this);
		newAttack->SetPos(pos);
	}
	else if (magicType == "meteor") {
		VECTOR mpos = pos;
		mpos.y += 600.0f;
		newAttack = new MeteorAttack(-1, true, vel, 2.0f, damage, this);
		newAttack->SetPos(mpos);
	}
	else if (magicType == "sword") {
		VECTOR spos = pos;
		spos.z += 120.0f;
		newAttack = new SwordAttack(-1, true, vel, 0.6f, damage, this);
		newAttack->SetPos(spos);
	}
	else if (magicType == "ice") {
		// ※IceAttackクラスを作成したらここを書き換えてください
		newAttack = new ThunderAttack(-1, true, vel, 1.5f, damage, this);
		newAttack->SetPos(pos);
	}
	else if (magicType == "shoot" || magicType == "thunder") {
		if (enemyList_ && !enemyList_->empty()) {
			for (auto* actor : *enemyList_) {
				if (actor && actor->IsEnemy()) {
					VECTOR epos = actor->GetPos();
					epos.y += 150.0f;
					vel = { 0.0f, -100.0f, 0.0f };
					int gridIdx = AttackBase::CalcGridIndex(epos, false);
					newAttack = new ThunderAttack(gridIdx, true, vel, 2.0f, damage, this);
					newAttack->SetPos(epos);
					break; // 最初の敵に落とす
				}
			}
		}
	}
	else if (magicType == "attack" || magicType == "ranged") {
		VECTOR rpos = pos;
		rpos.y += 80.0f;
		rpos.z += 160.0f;
		newAttack = new RangedAttack(-1, true, vel, 3.0f, damage, this);
		newAttack->SetPos(rpos);
	}
	else {
		// デフォルト：究極魔法
		newAttack = new UltimateAttack(-1, true, vel, 1.0f, damage, this);
		newAttack->SetPos(pos);
	}

	// 攻撃をマネージャーに登録
	if (newAttack && attackManager_) {
		attackManager_->Add(newAttack);
		Application::GetInstance()->ShakeScreen(40, 40, true, true);
		isAttacking_ = true;
	}
}

// =======================================================
// 移動・コマンド入力受付（攻撃は ExecuteMagic に一任）
// =======================================================
void Player::Move(void)
{
	if (isInputActive_) {
		GetKeyInputString(inputBuf_, keyInputHandle_);

		if (CheckKeyInput(keyInputHandle_) == 1) {

			std::string rawInput(inputBuf_);
			inputText_ = rawInput;

			DeleteKeyInput(keyInputHandle_);
			keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandle_);
			inputBuf_[0] = '\0';

			if (rawInput.empty()) return;

			std::string commandKey = ConvertIfRomanji(ToLowerTrim(rawInput));

			// 1. 必殺技リスト(Ultimate.csv)から検索
			std::string commandId;
			for (const auto& pair : attackManager_->registeredCommands_) {
				if (pair.first == commandKey) {
					commandId = pair.second;
					break;
				}
			}

			if (!commandId.empty()) {
				auto itUltimate = attackManager_->ultimateCommandDataMap_.find(commandId);
				if (itUltimate != attackManager_->ultimateCommandDataMap_.end()) {
					const auto& data = itUltimate->second;

					// ★修正ポイント1：Ultimate.csvに「ICE」などと書いた場合、それをそのまま属性にする！
					std::string typeLower = ToLowerTrim(commandId);

					auto itMagic = magicTypeMap_.find(commandKey);
					if (itMagic != magicTypeMap_.end() && !itMagic->second.empty()) {
						typeLower = itMagic->second; // Word.csv にも登録があればそっちを優先
					}

					// 魔法発動！
					ExecuteMagic(typeLower, data.damage, data.speed);
				}
				return;
			}

			// 2. 通常コマンドリスト(Word.csv)から検索
			auto it = commandMap_.find(commandKey);
			if (it != commandMap_.end()) {
				CommandType type = it->second;

	
				if (type == CommandType::UNKNOWN) {
					std::string mType = magicTypeMap_[commandKey];
					// CSVにダメージ列がないので、仮でダメージ20・速度10として渡す
					ExecuteMagic(mType, 20, 10.0f);
					return;
				}

				switch (type) {
				case CommandType::MOVE_UP:
					MoveToGrid({ gridPos_.x, gridPos_.z + 1 });
					break;
				case CommandType::MOVE_DOWN:
					MoveToGrid({ gridPos_.x, gridPos_.z - 1 });
					break;
				case CommandType::MOVE_LEFT:
					MoveToGrid({ gridPos_.x - 1, gridPos_.z });
					break;
				case CommandType::MOVE_RIGHT:
					MoveToGrid({ gridPos_.x + 1, gridPos_.z });
					break;
				case CommandType::MOVE_UP_RIGHT:
					MoveToGrid({ gridPos_.x + 1, gridPos_.z + 1 });
					break;
				case CommandType::MOVE_UP_LEFT:
					MoveToGrid({ gridPos_.x - 1, gridPos_.z + 1 });
					break;
				case CommandType::MOVE_DOWN_RIGHT:
					MoveToGrid({ gridPos_.x + 1, gridPos_.z - 1 });
					break;
				case CommandType::MOVE_DOWN_LEFT:
					MoveToGrid({ gridPos_.x - 1, gridPos_.z - 1 });
					break;
				case CommandType::DODGE:
				case CommandType::MOVE_RANDOM:
				{
					static const int dxTable[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
					static const int dzTable[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
					static std::random_device rd;
					static std::mt19937 mt(rd());
					static std::uniform_int_distribution<int> dist(0, 7);
					int dir = dist(mt);
					GridPos next = gridPos_;
					next.x += dxTable[dir];
					next.z += dzTable[dir];
					MoveToGrid(next);
				}
				break;

				case CommandType::SHOOT:
					ExecuteMagic("shoot", 10, 10.0f);
					break;
				case CommandType::ATTACK:
					ExecuteMagic("attack", 5, 10.0f);
					break;
				default:
					break;
				}
			}
		}
	}
}
void Player::ApplyDamage(int damage) {
	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
}

void Player::AddStun(int value) {
	stunGauge_ += value;
	if (stunGauge_ > maxStunGauge_) stunGauge_ = maxStunGauge_;
}

void Player::OnStunned() {
}

bool Player::IsDead() const {
	return hp_ <= 0;
}

void Player::ChangeState(ActorState state) {
	state_ = state;
}

ActorBase::ActorState Player::GetState() const {
	return state_;
}

std::vector<std::string> Player::GetNormalCommandNames() const {
	std::vector<std::string> names;
	for (const auto& pair : commandMap_) {
		names.push_back(pair.first);
	}
	return names;
}