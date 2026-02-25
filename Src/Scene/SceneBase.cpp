#include "SceneBase.h"

SceneBase::SceneBase(void) {}
SceneBase::~SceneBase(void) {} // 純粋仮想デストラクタでも定義が必要

void SceneBase::Init(void) {}
void SceneBase::Load(void) {}
void SceneBase::Update(void) {}
void SceneBase::Draw(void) {}
void SceneBase::Release(void) {}