#if 0

#include "TransitionEffect.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//TransitionEffect
**********************************************************/
TransitionEffect::TransitionEffect()
{
}
TransitionEffect::~TransitionEffect()
{
}

/**********************************************************
//TransitionEffect_FadeOut
**********************************************************/
void TransitionEffect_FadeOut::Work()
{
	if (sprite_ == NULL)
		return;
	alpha_ -= diffAlpha_;
	alpha_ = _MAX(alpha_, 0);
	sprite_->SetAlpha((int)alpha_);
}
void TransitionEffect_FadeOut::Render()
{
	if (sprite_ == NULL)
		return;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(DirectGraphics::BlendMode::Alpha);
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnable(false);
	sprite_->Render();
}
bool TransitionEffect_FadeOut::IsEnd()
{
	bool res = (alpha_ <= 0);
	return res;
}
void TransitionEffect_FadeOut::Initialize(int frame, std::shared_ptr<Texture> texture)
{
	diffAlpha_ = 255.0 / frame;
	alpha_ = 255.0;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	auto width = graphics->GetRenderWidth();
	auto height = graphics->GetRenderHeight();
	RECT_D rect = { 0., 0., width, height };

	sprite_ = new Sprite2D();
	sprite_->SetTexture(texture);
	sprite_->SetVertex(rect, rect, D3DCOLOR_ARGB((int)alpha_, 255, 255, 255));
}

/**********************************************************
//TransitionEffectTask
**********************************************************/
TransitionEffectTask::TransitionEffectTask()
{
}
TransitionEffectTask::~TransitionEffectTask()
{
}
void TransitionEffectTask::SetTransition(std::shared_ptr<TransitionEffect> effect)
{
	effect_ = effect;
}
void TransitionEffectTask::Work()
{
	if (effect_ != NULL) {
		effect_->Work();
	}
}
void TransitionEffectTask::Render()
{
	if (effect_ != NULL)
		effect_->Render();
}

#endif
