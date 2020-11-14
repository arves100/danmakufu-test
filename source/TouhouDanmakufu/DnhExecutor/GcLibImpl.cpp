#include "GcLibImpl.hpp"
#include "StgScene.hpp"
#include "System.hpp"

/**********************************************************
//EApplication
**********************************************************/
EApplication::EApplication()
{
}
EApplication::~EApplication()
{
}
bool EApplication::_Initialize()
{
	ELogger* logger = ELogger::GetInstance();
	Logger::WriteTop(L"アプリケーション初期化");

	EFileManager* fileManager = EFileManager::CreateInstance();
	fileManager->Initialize();

	EFpsController* fpsController = EFpsController::CreateInstance();

	std::string appName = u8"東方弾幕風 ph3 ";
	appName += DNH_VERSION;
	appName += " {NEW:" __TIMESTAMP__ "}";

	DnhConfiguration* config = DnhConfiguration::CreateInstance();
	std::string configWindowTitle = config->GetWindowTitle();
	if (configWindowTitle.size() > 0)
		appName = configWindowTitle;

	//マウス表示
	if (!config->IsMouseVisible())
		WindowUtility::SetMouseVisible(false);

	//DirectX初期化
	EDirectGraphics* graphics = EDirectGraphics::CreateInstance();
	graphics->Initialize();
	SDL_Window* hWndMain = graphics->GetWindowHandle();
#if 0
	WindowLogger::InsertOpenCommandInSystemMenu(hWndMain);
#endif
	SDL_SetWindowTitle(hWndMain, appName.c_str());

	// TODO: Icon
	//::SetClassLong(hWndMain, GCL_HICON, (LONG)LoadIcon(GetApplicationHandle(), MAKEINTRESOURCE(IDI_ICON)));

	ErrorDialog::SetParentWindowHandle(hWndMain);

	ETextureManager* textureManager = ETextureManager::CreateInstance();
	textureManager->Initialize();

	EShaderManager* shaderManager = EShaderManager::CreateInstance();
	shaderManager->Initialize();

	EMeshManager* meshManager = EMeshManager::CreateInstance();
	meshManager->Initialize();

	EDxTextRenderer* textRenderer = EDxTextRenderer::CreateInstance();
	textRenderer->Initialize();

	EDirectSoundManager* soundManager = EDirectSoundManager::CreateInstance();
	soundManager->Initialize(hWndMain);

	EDirectInput* input = EDirectInput::CreateInstance();
	input->Initialize(hWndMain);

	ETaskManager* taskManager = ETaskManager::CreateInstance();
	taskManager->Initialize();

#if 0
	gstd::ref_count_ptr<gstd::TaskInfoPanel> panelTask = new gstd::TaskInfoPanel();
	bool bAddTaskPanel = logger->AddPanel(panelTask, L"Task");
	if (bAddTaskPanel)
		taskManager->SetInfoPanel(panelTask);

	gstd::ref_count_ptr<directx::TextureInfoPanel> panelTexture = new directx::TextureInfoPanel();
	bool bTexturePanel = logger->AddPanel(panelTexture, L"Texture");
	if (bTexturePanel)
		textureManager->SetInfoPanel(panelTexture);

	gstd::ref_count_ptr<directx::DxMeshInfoPanel> panelMesh = new directx::DxMeshInfoPanel();
	bool bMeshPanel = logger->AddPanel(panelMesh, L"Mesh");
	if (bMeshPanel)
		meshManager->SetInfoPanel(panelMesh);

	gstd::ref_count_ptr<directx::SoundInfoPanel> panelSound = new directx::SoundInfoPanel();
	bool bSoundPanel = logger->AddPanel(panelSound, L"Sound");
	if (bSoundPanel)
		soundManager->SetInfoPanel(panelSound);

	gstd::ref_count_ptr<gstd::ScriptCommonDataInfoPanel> panelCommonData = logger->GetScriptCommonDataInfoPanel();
	logger->AddPanel(panelCommonData, L"Common Data");

	gstd::ref_count_ptr<ScriptInfoPanel> panelScript = new ScriptInfoPanel();
	logger->AddPanel(panelScript, L"Script");

	if (config->IsLogWindow()) {
		logger->LoadState();
		logger->SetWindowVisible(true);
	}
#endif

	SystemController* systemController = SystemController::CreateInstance();
	systemController->Reset();

	Logger::WriteTop(L"アプリケーション初期化完了");

	return true;
}
bool EApplication::_Loop()
{
	ELogger* logger = ELogger::GetInstance();
	ETaskManager* taskManager = ETaskManager::GetInstance();
	EFpsController* fpsController = EFpsController::GetInstance();
	EDirectGraphics* graphics = EDirectGraphics::GetInstance();

	SDL_Window* hWndGraphics = graphics->GetWindowHandle();
#if 0
	SDL_Window* hWndLogger = ELogger::GetInstance()->GetWindowHandle();
#endif

	EDirectInput* input = EDirectInput::GetInstance();
	input->Update();
	if (input->GetKeyState(SDLK_LCTRL) == KEY_HOLD && input->GetKeyState(SDLK_LSHIFT) == KEY_HOLD && input->GetKeyState(SDLK_r) == KEY_PUSH) {
		//リセット
		SystemController* systemController = SystemController::CreateInstance();
		systemController->Reset();
	}

	taskManager->CallWorkFunction();

	if (!fpsController->IsSkip()) {
		graphics->BeginScene();
		taskManager->CallRenderFunction();
		graphics->EndScene();
	}

	fpsController->Wait();

	//ログ関連
#if 0
	SYSTEMTIME time;
	GetLocalTime(&time);
	std::wstring fps = StringUtility::Format(L"Work：%.2ffps、Draw：%.2ffps",
		fpsController->GetCurrentWorkFps(),
		fpsController->GetCurrentRenderFps());
	logger->SetInfo(0, L"fps", fps);

	int widthConfig = graphics->GetConfigData().GetScreenWidth();
	int heightConfig = graphics->GetConfigData().GetScreenHeight();
	int widthScreen = widthConfig * graphics->GetScreenWidthRatio();
	int heightScreen = heightConfig * graphics->GetScreenHeightRatio();

	std::wstring screen = StringUtility::Format(L"width：%d/%d、height：%d/%d",
		widthScreen, widthConfig,
		heightScreen, heightConfig);
	logger->SetInfo(1, L"screen", screen);

	logger->SetInfo(2, L"font cache",
		StringUtility::Format(L"%d", EDxTextRenderer::GetInstance()->GetCacheCount()));
#endif

	//高速動作
	int fastModeKey = fpsController->GetFastModeKey();
	if (input->GetKeyState(fastModeKey) == KEY_HOLD) {
		if (!fpsController->IsFastMode())
			fpsController->SetFastMode(true);
	} else if (input->GetKeyState(fastModeKey) == KEY_PULL || input->GetKeyState(fastModeKey) == KEY_FREE) {
		if (fpsController->IsFastMode())
			fpsController->SetFastMode(false);
	}
	return true;
}
void EApplication::WindowEvent(SDL_Event* evt)
{
	EDirectGraphics::GetInstance()->EventProcedure(evt);
}
bool EApplication::_Finalize()
{
	Logger::WriteTop(L"アプリケーション終了処理開始");
	SystemController::DeleteInstance();
	ETaskManager::DeleteInstance();
	EFileManager::GetInstance()->EndLoadThread();
	EDirectInput::DeleteInstance();
	EDirectSoundManager::DeleteInstance();
	EDxTextRenderer::DeleteInstance();
	EMeshManager::DeleteInstance();
	EShaderManager::DeleteInstance();
	ETextureManager::DeleteInstance();
	EDirectGraphics::DeleteInstance();
	EFpsController::DeleteInstance();
	EFileManager::DeleteInstance();

#if 0
	ELogger* logger = ELogger::GetInstance();
	logger->SaveState();
#endif

	Logger::WriteTop(L"アプリケーション終了処理完了");
	return true;
}

/**********************************************************
//EDirectGraphics
**********************************************************/
EDirectGraphics::EDirectGraphics()
{
}
EDirectGraphics::~EDirectGraphics()
{
}
bool EDirectGraphics::Initialize()
{
	DnhConfiguration* dnhConfig = DnhConfiguration::GetInstance();
	int screenWidth = dnhConfig->GetScreenWidth();
	int screenHeight = dnhConfig->GetScreenHeight();
	int screenMode = dnhConfig->GetScreenMode();
	int windowSize = dnhConfig->GetWindowSize();

	bool bUserSize = screenWidth != 640 || screenHeight != 480;
	if (!bUserSize) {
		if (windowSize == DnhConfiguration::WINDOW_SIZE_640x480 && screenWidth > 640)
			windowSize = DnhConfiguration::WINDOW_SIZE_800x600;
		if (windowSize == DnhConfiguration::WINDOW_SIZE_800x600 && screenWidth > 800)
			windowSize = DnhConfiguration::WINDOW_SIZE_960x720;
		if (windowSize == DnhConfiguration::WINDOW_SIZE_960x720 && screenWidth > 960)
			windowSize = DnhConfiguration::WINDOW_SIZE_1280x960;
		if (windowSize == DnhConfiguration::WINDOW_SIZE_1280x960 && screenWidth > 1280)
			windowSize = DnhConfiguration::WINDOW_SIZE_1600x1200;
		if (windowSize == DnhConfiguration::WINDOW_SIZE_1600x1200 && screenWidth > 1600)
			windowSize = DnhConfiguration::WINDOW_SIZE_1920x1200;
	}

	bool bFullScreen = screenMode == DirectGraphics::SCREENMODE_FULLSCREEN;

	DirectGraphicsConfig dxConfig;
	dxConfig.SetScreenWidth(screenWidth);
	dxConfig.SetScreenHeight(screenHeight);
	dxConfig.SetFullScreen(bFullScreen);
	bool res = DirectGraphicsPrimaryWindow::Initialize(dxConfig);

	//コンフィグ反映
	if (bFullScreen) {
		ChangeScreenMode();
	} else {
		if (windowSize != DnhConfiguration::WINDOW_SIZE_640x480 || bUserSize) {
			int width = screenWidth;
			int height = screenHeight;
			if (!bUserSize) {
				switch (windowSize) {
				case DnhConfiguration::WINDOW_SIZE_800x600:
					width = 800;
					height = 600; // + ::GetSystemMetrics(SM_CXEDGE) + 10 ; + ::GetSystemMetrics(SM_CXEDGE) + 10 ;
					break;
				case DnhConfiguration::WINDOW_SIZE_960x720:
					width = 960;
					height = 720; // + ::GetSystemMetrics(SM_CXEDGE) + 10 ; + ::GetSystemMetrics(SM_CXEDGE) + 10 ;
					break;
				case DnhConfiguration::WINDOW_SIZE_1280x960:
					width = 1280;
					height = 960; // + ::GetSystemMetrics(SM_CXEDGE) + 10 ; + ::GetSystemMetrics(SM_CXEDGE) + 10 ;
					break;
				case DnhConfiguration::WINDOW_SIZE_1600x1200:
					width = 1600;
					height = 1200; // + ::GetSystemMetrics(SM_CXEDGE) + 10 ; + ::GetSystemMetrics(SM_CXEDGE) + 10 ;
					break;
				case DnhConfiguration::WINDOW_SIZE_1920x1200:
					width = 1920;
					height = 1200; // + ::GetSystemMetrics(SM_CXEDGE) + 10 ; + ::GetSystemMetrics(SM_CXEDGE) + 10 ;
					break;
				}
			}

			SetBounds(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height);
			MoveWindowCenter();
		}
	}
	SetWindowVisible(true);

	return res;
}
void EDirectGraphics::SetRenderStateFor2D(int blend)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(blend);
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
}
void EDirectGraphics::EventProcedure(SDL_Event* evt)
{
	// Did they just ?
#if 0
	switch (uMsg) {
	case WM_SYSCOMMAND:
		int nId = wParam & 0xffff;
		if (nId == WindowLogger::MENU_ID_OPEN)
			ELogger::GetInstance()->ShowLogWindow();
		break;
	}
#endif

	if (bShutdown_)
		EApplication::GetInstance()->End();

	DirectGraphicsPrimaryWindow::EventProcedure(evt);
}
