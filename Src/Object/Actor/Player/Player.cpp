#include "Player.h"

#include<DxLib.h>
#include <random> 
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../../../Application.h"
#include "../../../Input/InputManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../Common/AnimationController.h"
#include"../../Attack/AttackManager.h"
#include "../../Attack/RangedAttack/RangedAttack.h"

#include "../../../Camera/Camera.h"

Player::Player(Camera* camera)
{
	camera_ = camera;
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
	if (str == "MOVE") return CommandType::MOVE;
	if (str == "ATTACK") return CommandType::ATTACK;
	return CommandType::UNKNOWN;
}


void Player::InitLoad(void)
{
	// モデルの読み込み
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Player/Player.mv1").c_str());
	LoadMoveWordDict("Data/CSV/Word.csv");

	keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
	SetActiveKeyInput(keyInputHandle_);
	isInputActive_ = true;
	inputBuf_[0] = '\0';
	hiraText_.clear();
	romanjiConverter_.clear();
}

void Player::InitTransform(void)
{

	logicPos_ = { 0.0f, 0.0f, 0.0f };
	drawPos_ = logicPos_;
	gridPos_ = { 0, 0 };
	targetGridPos_ = gridPos_;
	// モデルの角度
	angle_ = { 0.0f, 0.0f, 0.0f };
	localAngle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f };

	// 角度から方向に変換する
	moveDir_ = { sinf(angle_.y), 0.0f, cosf(angle_.y) };
	preInputDir_ = moveDir_;

	// 行列の合成(子, 親と指定すると親⇒子の順に適用される)
	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);

	// 回転行列をモデルに反映
	MV1SetRotationMatrix(modelId_, mat);

	// モデルの位置設定
	pos_ = AsoUtility::VECTOR_ZERO;
	MV1SetPosition(modelId_, pos_);

	// 当たり判定を作成
	startCapsulePos_ = { 0.0f,110,0.0f };
	endCapsulePos_ = { 0.0f,30.0f,0.0f };
	capsuleRadius_ = 20.0f;
	
	// 当たり判定を取るか
	isCollision_ = true;

	
}

void Player::InitAnimation(void)
{
	// モデルアニメーション制御の初期化
	animationController_ = new AnimationController(modelId_);

	// アニメーションの追加
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::IDLE), 0.5f, Application::PATH_MODEL + "Player/Idle.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::WALK), 0.4f, Application::PATH_MODEL + "Player/FastRun.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::ATTACK2), 0.4f, Application::PATH_MODEL + "Player/Attack2.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::ATTACK3), 0.4f, Application::PATH_MODEL + "Player/Attack3.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::BOXING), 0.4f, Application::PATH_MODEL + "Player/Boxing.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::FALLING), 0.4f, Application::PATH_MODEL + "Player/Falling.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::FLYING), 0.4f, Application::PATH_MODEL + "Player/Flying.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::JUMP), 0.4f, Application::PATH_MODEL + "Player/Jump.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::JUMP_ATTACK), 0.4f, Application::PATH_MODEL + "Player/JumpAttack.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::JUMP_HOVER), 0.4f, Application::PATH_MODEL + "Player/JumpHover.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::JUMPING), 0.4f, Application::PATH_MODEL + "Player/Jumping.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::JUMP_RISING), 0.4f, Application::PATH_MODEL + "Player/JumpRising.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::SHOT), 0.4, Application::PATH_MODEL + "Player/Shot.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::THROW), 0.4f, Application::PATH_MODEL + "Player/Throw.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::VICTORY), 0.4f, Application::PATH_MODEL + "Player/Victory.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::WALK_LOOP), 0.4f, Application::PATH_MODEL + "Player/Walk.mv1");
	animationController_->Add(
		static_cast<int>(ANIM_TYPE::WARP_POSE), 0.4f, Application::PATH_MODEL + "Player/WarpPose.mv1");


	//animationController_->Add(
		//static_cast<int>(ANIM_TYPE::PUNCH), 0.5f, Application::PATH_MODEL + "Player/Punch.mv1");
	// 初期アニメーションの再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{

	// カメラに自分を追従させる
	if (camera_) {
		camera_->SetFollow(this);
	}
}

void Player::Update(void)
{
	ActorBase::Update();

	// グリッド移動処理
	if (isMovingOnGrid_) {
		// 目標グリッド座標をワールド座標に変換
		VECTOR targetLogicPos = {
			targetGridPos_.x * gridSize_,
			0.0f,
			targetGridPos_.z * gridSize_
		};

		// --- 進行方向を計算し、Y軸回転角を更新 ---
		VECTOR diff = {
			targetLogicPos.x - logicPos_.x,
			0.0f,
			targetLogicPos.z - logicPos_.z
		};
		// atan2(進行方向z, 進行方向x) だとX軸基準なので、Z軸基準に合わせる
		if (diff.x != 0.0f || diff.z != 0.0f) {
			angle_.y = atan2f(diff.x, diff.z);
		}

		// 線形補間でゆっくり移動
		float speed = 10.0f; // 1フレームの移動量
		float dist = sqrtf(diff.x * diff.x + diff.z * diff.z);
		if (dist < speed) {
			logicPos_.x = targetLogicPos.x;
			logicPos_.z = targetLogicPos.z;
			gridPos_ = targetGridPos_;
			isMovingOnGrid_ = false; // 到着
		}
		else {
			logicPos_.x += diff.x / dist * speed;
			logicPos_.z += diff.z / dist * speed;
		}
	}

	// アニメーションの更新
	animationController_->Update();


	// 攻撃アニメーション中は他のアニメーションを再生しない
	if (isAttacking_) {
		// 攻撃アニメーション開始時のみ再生指示
		if (animationController_->GetPlayType() != static_cast<int>(ANIM_TYPE::SHOT)) {
			animationController_->Play(static_cast<int>(ANIM_TYPE::SHOT), false); // ループしない
		}
		// アニメーションが終了したら攻撃状態解除
		if (animationController_->IsEnd()) {
			isAttacking_ = false;
		}
	}
	else {
		// 通常のアニメーション制御
		if (isMovingOnGrid_) {
			animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
		}
		else {
			animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
		}
	}
	// 進行方向にモデルを回転させる
	MATRIX mat = MatrixUtility::Multiplication(localAngle_, angle_);
	MV1SetRotationMatrix(modelId_, mat);

	pos_ = logicPos_;
}



void Player::Draw(void)
{
	ActorBase::Draw();

	DrawFormatString(
		0, 50, 0xffffff,
		"キャラ角度　 ：(%.1f, %.1f, %.1f)",
		AsoUtility::Rad2DegF(angle_.x),
		AsoUtility::Rad2DegF(angle_.y),
		AsoUtility::Rad2DegF(angle_.z)
	);
	if (isInputActive_) {
		DrawBox(0, 100, 400, 130, 0x222222, TRUE);
		DrawFormatString(10, 105, 0xff00ff, "コマンド入力(ローマ字): %s", inputBuf_);

		// 入力中のひらがな変換
		RomanjiConverter tmpConverter;
		tmpConverter.clear();
		for (int i = 0; inputBuf_[i] != '\0'; ++i) {
			tmpConverter.addInput(inputBuf_[i]);
		}
		std::string hiraText = tmpConverter.getOutput();

		DrawFormatString(10, 125, 0x00ff00, "変換(ひらがな): %s", hiraText.c_str());
	}

	// 前回入力のひらがな変換
	RomanjiConverter prevConverter;
	prevConverter.clear();
	for (int i = 0; inputText_[i] != '\0'; ++i) {
		prevConverter.addInput(inputText_[i]);
	}
	std::string prevHiraText = prevConverter.getOutput();

	DrawFormatString(10, 140, 0x00ff00, "直前の入力: %s（%s）", inputText_.c_str(), prevHiraText.c_str());

	// 弾発射デバッグ表示
	if (isBulletFired_) {
		DrawFormatString(10, 160, 0x00ff00, "弾を発射しました！");
	}

}



void Player::Release(void)
{
	ActorBase::Release();
}

void Player::LoadMoveWordDict(const std::string& path)
{
	std::ifstream file(path);
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string word, typeStr;
		if (std::getline(iss, word, ',') &&
			std::getline(iss, typeStr, ',')) {
			CommandType type = StringToCommandType(typeStr);
			commandMap_[word] = type;
		}
	}
}

void Player::Move(void)
{
	if (CheckHitKey(KEY_INPUT_F1))
	{
		VECTOR pos = GetPos();
		VECTOR vel = { 0, 0, 20 };
		int damage = 5;
		if (attackManager_) {
			attackManager_->Add(new RangedAttack(pos, vel, damage));
			isBulletFired_ = true;
		}
	}
	// 入力中
	if (isInputActive_) {
		GetKeyInputString(inputBuf_, keyInputHandle_);

		// 入力確定（Enterキー押下）
		if (CheckKeyInput(keyInputHandle_) == 1) {
			DeleteKeyInput(keyInputHandle_);
			SetActiveKeyInput(-1);
			isInputActive_ = false;

			// ローマ字→ひらがな変換
			romanjiConverter_.clear();
			for (int i = 0; inputBuf_[i] != '\0'; ++i) {
				romanjiConverter_.addInput(inputBuf_[i]);
			}
			hiraText_ = romanjiConverter_.getOutput();

			std::string command(inputBuf_);
			auto it = commandMap_.find(command);
			CommandType type = CommandType::UNKNOWN;
			if (it != commandMap_.end()) {
				type = it->second;
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
			{
				// 回避処理
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
				if (attackManager_) {
					VECTOR pos = GetPos();
					VECTOR vel = { 0, 0, 20 };
					int damage = 5;
					attackManager_->Add(new RangedAttack(pos, vel, damage));
					isBulletFired_ = true;
					isAttacking_ = true;
				}
				break;
			case CommandType::ATTACK:
				if (attackManager_) {
					VECTOR pos = GetPos();
					pos.y += 80; // 少し上から発射
					pos.z += 60; // 少し前から発射
					VECTOR vel = { 0, 0, 10 };
					int damage = 5;
					attackManager_->Add(new RangedAttack(pos, vel, damage));
					isBulletFired_ = true;
					isAttacking_ = true;
				}
				break;
			case CommandType::MOVE:
			{
				// moveWordDict_ で座標取得して移動
				auto it2 = moveWordDict_.find(command);
				if (it2 != moveWordDict_.end()) {
					int dx = it2->second.first;
					int dz = it2->second.second;
					GridPos next = gridPos_;
					next.x += dx;
					next.z += dz;
					MoveToGrid(next);
				}
			}
			break;
			default:
				break;
			}

			inputText_ = inputBuf_;
			keyInputHandle_ = MakeKeyInput(127, FALSE, FALSE, FALSE, FALSE);
			SetActiveKeyInput(keyInputHandle_);
			isInputActive_ = true;
			inputBuf_[0] = '\0';
			hiraText_.clear();
			romanjiConverter_.clear();
		}
	}

}


