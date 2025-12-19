#include <DxLib.h>
#include "Grid.h"

Grid::Grid(void)
{
}

Grid::~Grid(void)
{
}

void Grid::Init(void)
{
}

void Grid::Update(void)
{
}

void Grid::Draw(void)
{

	// XZ基本軸(グリッド)
	float num;
	VECTOR sPos;
	VECTOR ePos;
	for (int i = -HNUM; i < HNUM; i++)
	{

		num = static_cast<float>(i);

		// X軸(赤)
		sPos = { -HLEN, 0.0f, num * TERM };
		ePos = {  HLEN, 0.0f, num * TERM };
		DrawLine3D(sPos, ePos, 0xff0000);
		DrawSphere3D(ePos, 20.0f, 10, 0xff0000, 0xff0000, true);

		// Z軸(青)
		sPos = { num * TERM, 0.0f, -HLEN };
		ePos = { num * TERM, 0.0f,  HLEN };
		DrawLine3D(sPos, ePos, 0x0000ff);
		DrawSphere3D(ePos, 20.0f, 10, 0x0000ff, 0x0000ff, true);

	}

	//// Y軸(緑)
	//sPos = { 0.0f, -HLEN, 0.0f };
	//ePos = { 0.0f,  HLEN, 0.0f };
	//DrawLine3D(sPos, ePos, 0x00ff00);
	//DrawSphere3D(ePos, 20.0f, 10, 0x00ff00, 0x00ff00, true);

}

void Grid::Release(void)
{
}
VECTOR Grid::GetWorldPosFromIndex(int index, bool isPlayerSide)
{
	// グリッドは横5分割、左から0,1,2,3,4
	// プレイヤー側と敵側でZ座標を分ける場合は適宜調整
	constexpr int GRID_NUM = 5;
	constexpr float GRID_WIDTH = 400.0f;
	constexpr float GRID_ORIGIN_X = -800.0f; // 例: 5マス分の中心が原点になるように
	constexpr float PLAYER_Z = 0.0f;
	constexpr float ENEMY_Z = 800.0f;

	float x = GRID_ORIGIN_X + index * GRID_WIDTH;
	float y = 0.0f;
	float z = isPlayerSide ? PLAYER_Z : ENEMY_Z;

	VECTOR pos = { x, y, z };
	return pos;
}