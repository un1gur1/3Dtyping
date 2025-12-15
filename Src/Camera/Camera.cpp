#include "Camera.h"

#include "../Utility/AsoUtility.h"
#include "../Input/InputManager.h"

#include "../Object/Actor/ActorBase.h"

Camera::Camera(void)
{
	// DxLibの初期設定では、
	// カメラの位置が x = 320.0f, y = 240.0f, z = (画面のサイズによって変化)、
	// 注視点の位置は x = 320.0f, y = 240.0f, z = 1.0f
	// カメラの上方向は x = 0.0f, y = 1.0f, z = 0.0f
	// 右上位置からZ軸のプラス方向を見るようなカメラ
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	// カメラの初期位置
	pos_ = DEFAULT_POS;

	// カメラの初期角度
	angle_ = DERFAULT_ANGLES;
}

void Camera::Update(void)
{


}

void Camera::SetBeforeDraw(void)
{
	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	}
}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// カメラの設定(位置と角度による制御)
	SetCameraPositionAndAngle(
		pos_,
		angle_.x,
		angle_.y,
		angle_.z
	);
}

void Camera::SetBeforeDrawFree(void)
{
	// ゲームパッドが接続数で処理を分ける
	if (GetJoypadNum() == 0)
	{
		// 方向回転によるXYZの移動
		MoveXYZDirection();
	}
	else
	{
		// 方向回転によるXYZの移動(ゲームパッド)
		MoveXYZDirectionPad();
	}

	// カメラの設定(位置と角度による制御)
	SetCameraPositionAndAngle(
		pos_,
		angle_.x,
		angle_.y,
		angle_.z
	);
}

void Camera::DrawDebug(void)
{

	DrawFormatString(
		0, 10, 0xffffff,
		"カメラ座標　 ：(%.1f, %.1f, %.1f)",
		pos_.x, pos_.y, pos_.z
	);
	DrawFormatString(
		0, 30, 0xffffff,
		"カメラ角度　 ：(%.1f, %.1f, %.1f)",
		AsoUtility::Rad2DegF(angle_.x),
		AsoUtility::Rad2DegF(angle_.y),
		AsoUtility::Rad2DegF(angle_.z)
	);

	DrawSphere3D(targetPos_, 20.0f, 10, 0xff0000, 0xff0000, true);

}

void Camera::SetBeforeDrawFollow(void)
{

	if (GetJoypadNum() == 0)
	{
		// 方向回転によるXYZの移動
		MoveXYZDirection();
	}
	else
	{
		// 方向回転によるXYZの移動(ゲームパッド)
		MoveXYZDirectionPad();
	}

	// カメラの回転行列を作成
	MATRIX mat = MGetIdent();
	mat = MMult(mat, MGetRotX(angle_.x));
	mat = MMult(mat, MGetRotY(angle_.y));
	//mat = MMult(mat, MGetRotZ(angles_.z));
	
	// カメラの回転行列(X抜き)を作成
	MATRIX matY = MGetIdent();
	//mat = MMult(mat, MGetRotX(angles_.x));
	matY = MMult(matY, MGetRotY(angle_.y));
	//mat = MMult(mat, MGetRotZ(angles_.z));

	// 注視点の移動
	VECTOR followPos = follow_->GetPos();
	//VECTOR targetLocalRotPos = VTransform(FOLLOW_TARGET_LOCAL_POS, matY);
	VECTOR targetLocalRotPos = VTransform(FOLLOW_TARGET_LOCAL_POS, mat);
	targetPos_ = VAdd(followPos, targetLocalRotPos);

	// カメラの移動
	// 相対座標を回転させて、回転後の相対座標を取得する
	VECTOR cameraLocalRotPos = VTransform(FOLLOW_CAMERA_LOCAL_POS, mat);

	// 相対座標からワールド座標に直して、カメラ座標とする
	pos_ = VAdd(followPos, cameraLocalRotPos);

	// カメラの上方向を計算
	VECTOR up = VTransform(AsoUtility::DIR_U, mat);

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		pos_,
		targetPos_,
		up
	);
}

void Camera::Release(void)
{
}

void Camera::ChangeMode(MODE mode)
{
	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		break;
	case Camera::MODE::FREE:
		break;
	case Camera::MODE::FOLLOW:
		break;
	}
}

void Camera::SetFollow(ActorBase* follow)
{
	follow_ = follow;
}

void Camera::MoveXYZDirection(void)
{
	// 矢印キーでカメラの角度を変える
	float rotPow = 1.0f * DX_PI_F / 180.0f;
	if (InputManager::GetInstance()->IsNew(KEY_INPUT_DOWN))	{ angle_.x += rotPow; }
	if (InputManager::GetInstance()->IsNew(KEY_INPUT_UP))	{ angle_.x -= rotPow; }
	if (InputManager::GetInstance()->IsNew(KEY_INPUT_RIGHT)) { angle_.y += rotPow; }
	if (InputManager::GetInstance()->IsNew(KEY_INPUT_LEFT))	{ angle_.y -= rotPow; }
}

void Camera::MoveXYZDirectionPad(void)
{
	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState = 
		InputManager::GetInstance()->GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	VECTOR dir = AsoUtility::VECTOR_ZERO;


	const float ROT_POW_DEG = 2.0f;
	const float rotPow = ROT_POW_DEG * DX_PI_F / 180.0f;

	// 右スティックの傾き
	dir = InputManager::GetInstance()->GetDirectionXZAKey(padState.AKeyRX, padState.AKeyRY);

	// 右スティック左右の傾き
	angle_.y += dir.x * rotPow;

	// 右スティック上下の傾き
	angle_.x += dir.z * rotPow;
}
