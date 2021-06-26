#pragma once

class CTestApp
{
public:
	CTestApp();
	~CTestApp();

	bool Initialize();
	void Loop();

private: // -- init stage --
	bool InitSDL();
	bool InitEngine();

protected:
	// Utility
	directx::DxAllocator m_alloc;

	// Graphics
	directx::DirectGraphics m_graph;
	directx::DirectGraphicsConfig m_config;
	directx::DirectInput m_input;
	
	// Managers
	gstd::FileManager m_fileManager;
	directx::TextureManager m_texManager;
	directx::ShaderManager m_shManager;

	bool m_quit;

private:
	// SDL objects
	SDL_Window* m_window;
	bool m_sdlInit;
	SDL_SysWMinfo m_winfo;

	// Other
	bool m_debugStats;
	int64_t m_timeOffset;

protected:
	virtual bool OnInit() { return false; }
	virtual bool OnPreInit() { return false; }
	virtual void OnRender() {}
	virtual void OnDestroy() {}
	virtual void OnLoop(const float) {}
	virtual void OnRender(const float) {}
	virtual const char* GetTestName() const { return nullptr; }
};
