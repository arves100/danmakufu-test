// Danmakufu BGFX GcLibDx test application
// Copyright (C) 2021 Arves100

#include "StdAfx.h"
#include "TestApp.hpp"

CTestApp::CTestApp() : m_quit(false), m_window(nullptr), m_sdlInit(false), m_winfo(), m_debugStats(false), m_timeOffset(0)
{

}

CTestApp::~CTestApp()
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

bool CTestApp::Initialize()
{
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // detect memory leak at exit
#endif

	if (!OnPreInit())
		return false;

	if (!InitSDL())
		return false;

	if (!InitEngine())
		return false;

	if (!OnInit())
		return false;

	m_timeOffset = bx::getHPCounter();

	return true;
}

bool CTestApp::InitEngine()
{
	if (!m_fileManager.Initialize())
		return false;

#if SDL_VIDEO_DRIVER_WINDOWS
	if (!m_graph.Initialize(m_config, m_winfo.info.win.window, nullptr))
		return false;
#elif SDL_VIDEO_DRIVER_X11
	if (!m_graph.Initialize(m_config, m_winfo.info.x11.window, nullptr))
		return false;
#elif SDL_VIDEO_DRIVER_COCOA
	if (!m_graph.Initialize(m_config, m_winfo.info.cocoa.window, nullptr))
		return false;
#else
	#error "Unsupported platform!"
	return false;
#endif

	if (!m_texManager.Initialize())
		return false;

	if (!m_shManager.Initialize())
		return false;

	if (!m_input.Initialize())
		return false;

	m_graph.RestoreViews();

	return true;
}

bool CTestApp::InitSDL()
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

void CTestApp::Loop()
{
	SDL_Event evt;
	while (!m_quit)
	{
		if (SDL_PollEvent(&evt))
		{
			m_input.EventUpdate(&evt);

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
			m_input.Update();
			
			const float time = static_cast<float>((bx::getHPCounter() - m_timeOffset) / static_cast<double>(bx::getHPFrequency()));
			
			OnLoop(time);

			m_graph.BeginScene();
			
			OnRender(time);
			
			// Debug info text
			bgfx::dbgTextPrintf(0, 0, 0x0f, "commit: %s@%s %s", __COMMIT__, __BRANCH__,
#ifdef __TAG__
				"tag: " __TAG__
#else
				""
#endif
			);
			bgfx::dbgTextPrintf(0, 1, 0x0f, "BGFX API version: %u Render: %s", BGFX_API_VERSION, bgfx::getRendererName(bgfx::getRendererType()));
			bgfx::dbgTextPrintf(0, 2, 0x0f, "Test: %s", GetTestName());
				
			m_graph.EndScene();		

			m_timeOffset = time;

		}
	}
}
