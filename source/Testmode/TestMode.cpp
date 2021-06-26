// Danmakufu BGFX GcLibDx test application
// Copyright (C) 2021 Arves100

#include "../GcLib/directx/DirectGraphics.hpp"
#include "../GcLib/directx/Texture.hpp"
#include "../GcLib/directx/DxUtility.hpp"
#include "../GcLib/directx/Shader.hpp"
#include "../GcLib/directx/RenderObject.hpp"

#include <SDL2/SDL_syswm.h>
#include <Version.h>

// GcLibDx conversion of https://github.com/bkaradzic/bgfx/blob/master/examples/06-bump/bump.cpp
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

class CTestMode
{
public:
	CTestMode() : m_window(nullptr), m_sdlInit(false), m_winfo(), m_lights(4), m_quit(false), m_debugStats(false), m_timeOffset(0) {}

	virtual ~CTestMode()
	{
		if (m_window)
		{
			SDL_DestroyWindow(m_window);
			m_window = nullptr;
		}
		
		if (m_sdlInit)
		{
			SDL_Quit();
			m_sdlInit = false;
		}
	}
	
	bool Initialize()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // detect memory leak at exit

		if (!InitSDL())
			return false;

		if (!InitEngine())
			return false;

		if (!InitObjects())
			return false;

		return true;
	}

	void Loop()
	{
		SDL_Event evt;
		while (!m_quit)
		{
			if (SDL_PollEvent(&evt))
			{
				switch (evt.type)
				{
				case SDL_WINDOWEVENT:
					{
						switch (evt.window.type)
						{
						case SDL_WINDOWEVENT_CLOSE:
							m_quit = true;
							break;
						default:
							break;
						}
						break;
					}
				case SDL_KEYDOWN:
					switch (evt.key.keysym.scancode)
					{
					case SDL_SCANCODE_0:
						m_debugStats = !m_debugStats;
						bgfx::setDebug(m_debugStats ? BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT : BGFX_DEBUG_TEXT);
						break;
					}
					break;
					
				case SDL_QUIT:
					m_quit = true;
					break;
				default:
					break;
				}
			}
			else
			{
				m_graph.BeginScene();
				
				Render();

				// Debug info text
				bgfx::dbgTextPrintf(0, 0, 0x0f, "commit: %s@%s %s", __COMMIT__, __BRANCH__,
#ifdef __TAG__
					"tag: " __TAG__
#else
					""
#endif
				);
				bgfx::dbgTextPrintf(0, 1, 0x0f, "BGFX API version: %u Render: %s", BGFX_API_VERSION, bgfx::getRendererName(bgfx::getRendererType()));
				
				m_graph.EndScene();			

			}
		}
	}

private:
	void Render()
	{
		// delta time	
		const float time = static_cast<float>((bx::getHPCounter() - m_timeOffset) / static_cast<double>(bx::getHPFrequency()));

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
		
		//for (auto yy = 0; yy < 3; ++yy)
		{
			//for (auto xx = 0; xx < 3; ++xx)
			{
				m_obj.SetAngleXYZ(0.0f, 45.0f, 0.0f);
				m_obj.SetPosition(-3.0f + static_cast<float>(0) * 8.0f, -3.0f + static_cast<float>(0) * 8.0f, 3.0f * 1);
				m_obj.Submit();
			}
		}
	}
	
	bool InitSDL()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
			return false;

		m_sdlInit = true;
		
		m_window = SDL_CreateWindow("Danmakufu/BGFX Engine test application", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_config.RenderWidth, m_config.RenderHeight, SDL_WINDOW_SHOWN);
		if (!m_window)
			return false;

		SDL_VERSION(&m_winfo.version);

		if (!SDL_GetWindowWMInfo(m_window, &m_winfo))
			return false;
		
		return true;
	}

	bool InitEngine()
	{
		if (!m_fileManager.Initialize())
			return false;

		m_config.Render = bgfx::RendererType::Direct3D11;
		//m_config.Color = directx::DirectGraphicsConfig::ColorMode::Bit16;
		
		if (!m_graph.Initialize(m_config, m_winfo.info.win.window, nullptr)) // TODO: win32 only ...
			return false;
		
		if (!m_texManager.Initialize())
			return false;

		if (!m_shManager.Initialize())
			return false;

		m_graph.RestoreViews();

		return true;
	}

	bool InitObjects()
	{
		if (!m_obj.Initialize("BumpCube", "vs_bump.bin", "fs_bump.bin"))
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
		auto txt = std::make_unique<directx::Texture>();
		if (!txt->CreateFromFile("fieldstone-rgba.tga"))
			return false;
		m_obj.SetTexture("s_texColor", txt);

		// Init normal texture
		txt = std::make_unique<directx::Texture>();
		if (!txt->CreateFromFile("fieldstone-n.tga"))
			return false;
		m_obj.SetTexture("s_texNormal", txt, 1);

		if (!InitObjShaders())
			return false;

		m_timeOffset = bx::getHPCounter();

		return true;
	}

	bool InitObjShaders()
	{
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

		auto cam = m_graph.GetCamera();
		cam->SetRadius(0.0f);
		cam->SetAzimuthAngle(0.0f);
		cam->SetElevationAngle(0.0f);
		cam->SetFocus(20.0f, 0.0f, -18.0f);
		cam->SetRoll(7.0f);
		cam->SetProjectionMatrix(m_config.RenderWidth, m_config.RenderHeight, 0.1f, 100.0f, 60.0f);
		
		return true;
	}
	
	// Utility
	directx::DxAllocator m_alloc;

	// Graphics
	directx::DirectGraphics m_graph;
	directx::DirectGraphicsConfig m_config;
	
	// Managers
	gstd::FileManager m_fileManager;
	directx::TextureManager m_texManager;
	directx::ShaderManager m_shManager;

	// Objects
	directx::RenderObject<directx::VERTEX_LTA> m_obj;

	// SDL objects
	SDL_Window* m_window;
	bool m_sdlInit;
	SDL_SysWMinfo m_winfo;

	// Other
	int16_t m_lights;
	bool m_quit, m_debugStats;
	int64_t m_timeOffset;
};


int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	CTestMode bump;
	if (!bump.Initialize())
		return EXIT_FAILURE;

	bump.Loop();
	return EXIT_SUCCESS;
}
