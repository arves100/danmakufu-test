#include "StdAfx.h"
#include "SpriteTest.hpp"

CSpriteTest::CSpriteTest() : s2(nullptr)
{

}

CSpriteTest::~CSpriteTest()
{

}

bool CSpriteTest::OnInit()
{
	s2 = new directx::Sprite2D();

	auto txt = std::make_shared<directx::Texture>();
	
	if (!txt->CreateFromFile("System_Title_Background.png"))
		return false;

	s2->SetTexture(txt, 0);

	float th = txt->GetHeight(), tr = txt->GetWidth();

	directx::RECT_F src;
	src.top = 0.0f;
	src.bottom = th;
	src.left = 0.0f;
	src.right = tr;
	s2->SetSourceRect(src);


#if 0
	s2->SetVertexUV(0, 0.0f, 1.0f);
	s2->SetVertexUV(1, 1.0f, 1.0f);
	s2->SetVertexUV(2, 0.0f, 0.0f);
	s2->SetVertexUV(3, 1.0f, 0.0f);
#endif

	float p = m_graph.GetRenderWidth();
	float m = m_graph.GetRenderHeight();
	src.right = 1.0f;
	src.bottom = 1.0f;
	src.left = -1.0f;
	src.top = -1.0f;
	//src.left = tr - p;
	//src.top = th - m;
	s2->SetDestinationRect(src);

	return true;
}

bool CSpriteTest::OnPreInit()
{
	m_config.Render = bgfx::RendererType::Direct3D11;
	return true;
}

void CSpriteTest::OnDestroy()
{
	if (s2)
		delete s2;

	s2 = nullptr;
}

void CSpriteTest::OnLoop(const float)
{

}

void CSpriteTest::OnRender(const float)
{
	m_graph.SetZWriteEnable(true);
	m_graph.SetDepthTest(DepthMode::Always_);
	m_graph.UpdateState();

	s2->Submit();
}

DEFINE_MAIN(CSpriteTest)
