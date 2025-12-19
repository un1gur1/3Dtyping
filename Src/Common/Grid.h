#pragma once
#include <DxLib.h>
class Grid
{

public:
	enum class GridState {
		Normal,
		Warning,
		Attack
	};

	static VECTOR GetWorldPosFromIndex(int index, bool isPlayerSide);

	// 線の長さ
	static constexpr float LEN = 3200.0f;

	// 線の長さの半分
	static constexpr float HLEN = LEN / 2.0f;

	// 線の間隔
	static constexpr float TERM = 400.0f;

	// 線の数
	static const int NUM = static_cast<int>(LEN / TERM);

	// 線の数の半分
	static const int HNUM = NUM / 2;

	// コンストラクタ
	Grid(void);

	// デストラクタ
	~Grid(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

};
