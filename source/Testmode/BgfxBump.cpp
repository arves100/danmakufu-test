// GcLibDx conversion of Bgfx 06 bump example
// Ref: https://github.com/bkaradzic/bgfx/blob/master/examples/06-bump/bump.cpp

#include "StdAfx.h"
#include "BgfxBump.hpp"

static directx::VERTEX_LTA s_cubeVertices[24] =
{
	{-1.0f,  1.0f,  1.0f, directx::encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, directx::encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f,  1.0f, directx::encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0,      0, 0x7fff },
	{ 1.0f, -1.0f,  1.0f, directx::encodeNormalRgba8(0.0f,  0.0f,  1.0f), 0, 0x7fff, 0x7fff },
	{-1.0f,  1.0f, -1.0f, directx::encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0,      0,      0 },
	{ 1.0f,  1.0f, -1.0f, directx::encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f, directx::encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0,      0, 0x7fff },
	{ 1.0f, -1.0f, -1.0f, directx::encodeNormalRgba8(0.0f,  0.0f, -1.0f), 0, 0x7fff, 0x7fff },
	{-1.0f,  1.0f,  1.0f, directx::encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, directx::encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0, 0x7fff,      0 },
	{-1.0f,  1.0f, -1.0f, directx::encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0,      0, 0x7fff },
	{ 1.0f,  1.0f, -1.0f, directx::encodeNormalRgba8(0.0f,  1.0f,  0.0f), 0, 0x7fff, 0x7fff },
	{-1.0f, -1.0f,  1.0f, directx::encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0,      0,      0 },
	{ 1.0f, -1.0f,  1.0f, directx::encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f, directx::encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0,      0, 0x7fff },
	{ 1.0f, -1.0f, -1.0f, directx::encodeNormalRgba8(0.0f, -1.0f,  0.0f), 0, 0x7fff, 0x7fff },
	{ 1.0f, -1.0f,  1.0f, directx::encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, directx::encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0, 0x7fff,      0 },
	{ 1.0f, -1.0f, -1.0f, directx::encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0,      0, 0x7fff },
	{ 1.0f,  1.0f, -1.0f, directx::encodeNormalRgba8(1.0f,  0.0f,  0.0f), 0, 0x7fff, 0x7fff },
	{-1.0f, -1.0f,  1.0f, directx::encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0,      0,      0 },
	{-1.0f,  1.0f,  1.0f, directx::encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f, directx::encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0,      0, 0x7fff },
	{-1.0f,  1.0f, -1.0f, directx::encodeNormalRgba8(-1.0f,  0.0f,  0.0f), 0, 0x7fff, 0x7fff },
};

static const uint16_t s_cubeIndices[36] =
{
	 0,  2,  1,
	 1,  2,  3,
	 4,  5,  6,
	 5,  7,  6,

	 8, 10,  9,
	 9, 10, 11,
	12, 13, 14,
	13, 15, 14,

	16, 18, 17,
	17, 18, 19,
	20, 21, 22,
	21, 23, 22,
};

CBgfxBump::CBgfxBump() : m_lights(4) {}
CBgfxBump::~CBgfxBump() {}

bool CBgfxBump::OnPreInit()
{
	m_config.Render = bgfx::RendererType::Direct3D11;
	return true;
}

bool CBgfxBump::OnInit()
{
	{ // Object init
		if (!m_obj.Initialize("BumpCube"))
			return false;

		auto shsh = m_obj.GetShader(); // Get shader
		if (!shsh->CreateFromFile("testmode/06bump", false)) // We can define custom shaders BEFORE the object goes into the rendering stage
			return false;

		// Primitive info
		directx::calcTangents(s_cubeVertices
				, BX_COUNTOF(s_cubeVertices)
				, m_obj.GetVertexDecl()
				, s_cubeIndices
				, BX_COUNTOF(s_cubeIndices)
				);
		
		m_obj.SetVertices(s_cubeVertices, sizeof(s_cubeVertices) / sizeof(directx::VERTEX_LTA));
		m_obj.SetIndices(s_cubeIndices, sizeof(s_cubeIndices) / sizeof(uint16_t));

		// Init bump texture
		auto txt = std::make_shared<directx::Texture>();
		if (!txt->CreateFromFile("fieldstone-rgba.tga"))
			return false;

		m_obj.SetTexture(txt, 0);

		// Init normal texture
		txt = std::make_shared<directx::Texture>();
		if (!txt->CreateFromFile("fieldstone-n.tga"))
			return false;

		m_obj.SetTexture(txt, 1);
	}

	{ // Object shader init
		auto shsh = m_obj.GetShader(); // Get shader

		// Set inner right
		const float lightRgbInnerR[4][4] =
		{
			{ 1.0f, 0.7f, 0.2f, 0.8f },
			{ 0.7f, 0.2f, 1.0f, 0.8f },
			{ 0.2f, 1.0f, 0.7f, 0.8f },
			{ 1.0f, 0.4f, 0.2f, 0.8f },
		};

		if (!shsh->AddParameter("u_lightRgbInnerR", bgfx::UniformType::Vec4, lightRgbInnerR, sizeof(lightRgbInnerR), m_lights))
			return false;

		// Create light position radius
		if (!shsh->AddParameter("u_lightPosRadius", bgfx::UniformType::Vec4, nullptr, 0, m_lights))
			return false;
	}
		
	auto cam = m_graph.GetCamera();
	//cam->SetProjectionMatrix(m_config.RenderWidth, m_config.RenderHeight, 0.1f, 100.0f, 60.0f);	

	return true;
}

void CBgfxBump::OnRender(const float time)
{
	float lightPosRadius[4][4];
	for (uint32_t ii = 0; ii < m_lights; ++ii)
	{
		lightPosRadius[ii][0] = bx::sin((time * (0.1f + ii * 0.17f) + ii * bx::kPiHalf * 1.37f)) * 3.0f;
		lightPosRadius[ii][1] = bx::cos((time * (0.2f + ii * 0.29f) + ii * bx::kPiHalf * 1.49f)) * 3.0f;
		lightPosRadius[ii][2] = -2.5f;
		lightPosRadius[ii][3] = 3.0f;
	}

	auto shader = m_obj.GetShader();
	auto param = shader->GetParameter("u_lightPosRadius");
	param->Set(lightPosRadius, sizeof(lightPosRadius), m_lights);

	m_graph.SetZWriteEnable(true);
	m_graph.SetDepthTest(true);
	m_graph.UpdateState();

	for (auto yy = 0; yy < 3; ++yy)
	{
		for (auto xx = 0; xx < 3; ++xx)
		{
			m_obj.SetAngleXYZ(time*0.023f + xx*0.21f, time*0.03f + yy*0.37f);
			m_obj.SetPosition(0.0f + float(xx)*3.0f, 0.0f + float(yy)*3.0f, 0.0f);
			

			m_obj.Submit();
		}
	}
	
	auto pos = m_graph.GetCamera()->GetFocusPosition();
	bgfx::dbgTextPrintf(0, 3, 0x0f, "Camera pos: %f %f %f", pos.x, pos.y, pos.z);
}

void CBgfxBump::OnDestroy() {}

void CBgfxBump::OnLoop(const float time)
{
	auto cam = m_graph.GetCamera();
	auto pos = cam->GetFocusPosition();

	if (m_input.GetKeyState(SDLK_q) == directx::KEY_HOLD)
		pos.x += 0.1f;
	
	if (m_input.GetKeyState(SDLK_w) == directx::KEY_HOLD)
		pos.y += 0.1f;

	if (m_input.GetKeyState(SDLK_e) == directx::KEY_HOLD)
		pos.z += 0.1f;

	if (m_input.GetKeyState(SDLK_a) == directx::KEY_HOLD)
		pos.x -= 0.1f;
	
	if (m_input.GetKeyState(SDLK_s) == directx::KEY_HOLD)
		pos.y -= 0.1f;

	if (m_input.GetKeyState(SDLK_d) == directx::KEY_HOLD)
		pos.z -= 0.1f;

	cam->SetFocus(pos);
	cam->UpdateDeviceWorldViewMatrix();
}

DEFINE_MAIN(CBgfxBump)
