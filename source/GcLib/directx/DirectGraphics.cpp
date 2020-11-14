#include "DirectGraphics.hpp"

#include "Texture.hpp"

using namespace gstd;
using namespace directx;

#include <SDL_syswm.h>

/**********************************************************
//DirectGraphicsConfig
**********************************************************/
DirectGraphicsConfig::DirectGraphicsConfig()
{
	widthScreen_ = 800;
	heightScreen_ = 600;
	bWindowed_ = true;
	bUseRef_ = false;
	colorMode_ = COLOR_MODE_32BIT;
	bUseTripleBuffer_ = true;
	bUseWaitTimer_ = false;
	bIsFullScreen_ = false;
}
DirectGraphicsConfig::~DirectGraphicsConfig()
{
}

/**********************************************************
//DirectGraphics
**********************************************************/
DirectGraphics* DirectGraphics::thisBase_ = NULL;

DirectGraphics::DirectGraphics()
{
	pDirect3D_ = NULL;
	pDevice_ = NULL;
	pBackSurf_ = NULL;
	pZBuffer_ = NULL;
	camera_ = new DxCamera();
	camera2D_ = new DxCamera2D();
}
DirectGraphics::~DirectGraphics()
{
	Logger::WriteTop(L"DirectGraphics：終了開始");

	if (pZBuffer_ != NULL)
		pZBuffer_->Release();
	if (pBackSurf_ != NULL)
		pBackSurf_->Release();
	if (pDevice_ != NULL)
		pDevice_->Release();
	if (pDirect3D_ != NULL)
		pDirect3D_->Release();
	thisBase_ = NULL;
	Logger::WriteTop(L"DirectGraphics：終了完了");
}
bool DirectGraphics::Initialize(SDL_Window* hWnd)
{
	return this->Initialize(hWnd, config_);
}
bool DirectGraphics::Initialize(SDL_Window* SDL_hWnd, DirectGraphicsConfig& config)
{
	if (thisBase_ != NULL)
		return false;

	// Get native window handle from an SDL2 window
	SDL_SysWMinfo info;

	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(SDL_hWnd, &info))
	{
		OutputDebugStringA(SDL_GetError()); // Not based ...
		throw gstd::wexception(L"Cannot get SDL window info");
	}

	HWND hWnd = info.info.win.window;

	Logger::WriteTop(L"DirectGraphics：初期化");
	pDirect3D_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (pDirect3D_ == NULL)
		throw gstd::wexception(L"Direct3DCreate9失敗");

	config_ = config;
	hAttachedWindow_ = SDL_hWnd;

	//FullScreenModeの設定
	ZeroMemory(&d3dppFull_, sizeof(D3DPRESENT_PARAMETERS));
	d3dppFull_.hDeviceWindow = hWnd;
	d3dppFull_.BackBufferWidth = config_.GetScreenWidth();
	d3dppFull_.BackBufferHeight = config_.GetScreenHeight();
	d3dppFull_.Windowed = FALSE;
	d3dppFull_.SwapEffect = D3DSWAPEFFECT_DISCARD;
	if (config_.GetColorMode() == DirectGraphicsConfig::COLOR_MODE_16BIT)
		d3dppFull_.BackBufferFormat = D3DFMT_R5G6B5;
	else
		d3dppFull_.BackBufferFormat = D3DFMT_X8R8G8B8;
	if (config_.IsTripleBufferEnable())
		d3dppFull_.BackBufferCount = 1;
	else
		d3dppFull_.BackBufferCount = 2;
	if (config_.IsWaitTimerEnable() == false)
		d3dppFull_.FullScreen_RefreshRateInHz = 60;
	d3dppFull_.EnableAutoDepthStencil = TRUE;
	d3dppFull_.AutoDepthStencilFormat = D3DFMT_D16;
	d3dppFull_.MultiSampleType = D3DMULTISAMPLE_NONE;

	//WindowModeの設定
	D3DDISPLAYMODE dmode;
	HRESULT hrAdapt = pDirect3D_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dmode);
	ZeroMemory(&d3dppWin_, sizeof(D3DPRESENT_PARAMETERS));
	d3dppWin_.BackBufferWidth = config_.GetScreenWidth();
	d3dppWin_.BackBufferHeight = config_.GetScreenHeight();
	d3dppWin_.Windowed = TRUE;
	d3dppWin_.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dppWin_.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dppWin_.hDeviceWindow = hWnd;
	d3dppWin_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (config_.IsTripleBufferEnable())
		d3dppWin_.BackBufferCount = 1;
	else
		d3dppWin_.BackBufferCount = 2;
	d3dppWin_.EnableAutoDepthStencil = TRUE;
	d3dppWin_.AutoDepthStencilFormat = D3DFMT_D16;
	d3dppWin_.MultiSampleType = D3DMULTISAMPLE_NONE;

	int countAdapt = pDirect3D_->GetAdapterCount();

	D3DPRESENT_PARAMETERS d3dpp = config_.IsWindowed() ? d3dppWin_ : d3dppFull_;
	modeScreen_ = config_.IsWindowed() ? SCREENMODE_WINDOW : SCREENMODE_FULLSCREEN;
	HRESULT hrDevice = -1;
	if (config_.IsReferenceEnable()) {
		hrDevice = pDirect3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &d3dpp, &pDevice_);
	} else {
		D3DCAPS9 caps;
		pDirect3D_->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
		if (caps.VertexShaderVersion >= D3DVS_VERSION(2, 0)) {
			hrDevice = pDirect3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &d3dpp, &pDevice_);
			if (!FAILED(hrDevice))
				Logger::WriteTop(L"DirectGraphics：デバイス初期化完了->D3DCREATE_HARDWARE_VERTEXPROCESSING");
			if (FAILED(hrDevice)) {
				hrDevice = pDirect3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &d3dpp, &pDevice_);
				if (!FAILED(hrDevice))
					Logger::WriteTop(L"DirectGraphics：デバイス初期化完了->D3DCREATE_SOFTWARE_VERTEXPROCESSING");
			}
		} else {
			hrDevice = pDirect3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &d3dpp, &pDevice_);
			if (!FAILED(hrDevice))
				Logger::WriteTop(L"DirectGraphics：デバイス初期化完了->D3DCREATE_SOFTWARE_VERTEXPROCESSING");
		}

		if (FAILED(hrDevice)) {
			Logger::WriteTop(L"DirectGraphics：HEL動作します。おそらく正常動作しません。");
			hrDevice = pDirect3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &d3dpp, &pDevice_);
		}
	}

	if (FAILED(hrDevice)) {
		throw gstd::wexception(L"IDirect3DDevice9::CreateDevice失敗");
	}

	// BackSurface取得
	pDevice_->GetRenderTarget(0, &pBackSurf_);

	// Zバッファ取得
	pDevice_->GetDepthStencilSurface(&pZBuffer_);

	thisBase_ = this;

	if (camera2D_ != NULL)
		camera2D_->Reset();
	_InitializeDeviceState();

	BeginScene();
	EndScene();

	Logger::WriteTop(L"DirectGraphics：初期化完了");
	return true;
}

void DirectGraphics::_ReleaseDxResource()
{
	if (pZBuffer_ != NULL)
		pZBuffer_->Release();
	if (pBackSurf_ != NULL)
		pBackSurf_->Release();
	std::list<DirectGraphicsListener*>::iterator itr;
	for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
		(*itr)->ReleaseDirectGraphics();
	}
}
void DirectGraphics::_RestoreDxResource()
{
	pDevice_->GetRenderTarget(0, &pBackSurf_);
	pDevice_->GetDepthStencilSurface(&pZBuffer_);
	std::list<DirectGraphicsListener*>::iterator itr;
	for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
		(*itr)->RestoreDirectGraphics();
	}
	_InitializeDeviceState();
}
void DirectGraphics::_Restore()
{
	Logger::WriteTop(L"DirectGraphics：_Restore開始");
	// ディスプレイの協調レベルを調査
	HRESULT hr = pDevice_->TestCooperativeLevel();
	if (hr == D3DERR_DEVICELOST) {
		int count = 0;
		do {
			Sleep(500); // 0.5秒待つ
			count += 500;
			hr = pDevice_->TestCooperativeLevel();
			if (hr == D3DERR_DEVICENOTRESET)
				break;
		} while (count < 6000);
	}

	// リストア
	_ReleaseDxResource();

	//デバイスリセット
	if (modeScreen_ == SCREENMODE_FULLSCREEN)
		pDevice_->Reset(&d3dppFull_);
	else
		pDevice_->Reset(&d3dppWin_);

	_RestoreDxResource();

	Logger::WriteTop(L"DirectGraphics：_Restore完了");
}
void DirectGraphics::_InitializeDeviceState()
{
	D3DXMATRIX viewMat;
	D3DXMATRIX persMat;
	if (camera_ != NULL) {
		camera_->UpdateDeviceWorldViewMatrix();
	} else {
		D3DVECTOR viewFrom = D3DXVECTOR3(100, 300, -500);
		D3DXMatrixLookAtLH(&viewMat, (D3DXVECTOR3*)&viewFrom, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));
	}
	D3DXMatrixPerspectiveFovLH(&persMat, D3DXToRadian(45.0),
		(float)config_.GetScreenWidth() / (float)config_.GetScreenHeight(), 10, 2000);

	pDevice_->SetTransform(D3DTS_VIEW, &viewMat);
	pDevice_->SetTransform(D3DTS_PROJECTION, &persMat);

	SetCullingMode(D3DCULL_NONE);
	pDevice_->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	pDevice_->SetRenderState(D3DRS_AMBIENT, RGB(192, 192, 192));
	SetLightingEnable(true);

	D3DVECTOR dir;
	dir.x = -1;
	dir.y = -1;
	dir.z = -1;
	SetDirectionalLight(dir);

	SetBlendMode(MODE_BLEND_ALPHA);

	//αテスト
	SetAlphaTest(true, 0, D3DCMP_GREATER);

	//Zテスト
	SetZBufferEnable(false);
	SetZWriteEnalbe(false);

	//Filter
	SetTextureFilter(MODE_TEXTURE_FILTER_LINEAR);

	//ViewPort
	ResetViewPort();
}
void DirectGraphics::AddDirectGraphicsListener(DirectGraphicsListener* listener)
{
	std::list<DirectGraphicsListener*>::iterator itr;
	for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
		if ((*itr) == listener)
			return;
	}
	listListener_.push_back(listener);
}
void DirectGraphics::RemoveDirectGraphicsListener(DirectGraphicsListener* listener)
{
	std::list<DirectGraphicsListener*>::iterator itr;
	for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
		if ((*itr) != listener)
			continue;
		listListener_.erase(itr);
		break;
	}
}
void DirectGraphics::BeginScene(bool bClear)
{
	if (bClear)
		ClearRenderTarget();
	pDevice_->BeginScene();
	camera_->UpdateDeviceWorldViewMatrix();
}
void DirectGraphics::EndScene()
{
	pDevice_->EndScene();

	HRESULT hr = pDevice_->Present(NULL, NULL, NULL, NULL);
	if (FAILED(hr)) {
		_Restore();
		_InitializeDeviceState();
	}
}
void DirectGraphics::ClearRenderTarget()
{
	int width = GetScreenWidth();
	int height = GetScreenWidth();
	D3DRECT rcDest = { 0, 0, width, height };
	if (textureTarget_ == NULL) {
		pDevice_->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0, 0);
	} else {
		pDevice_->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0, 0);
	}
}
void DirectGraphics::ClearRenderTarget(RECT rect)
{
	D3DRECT rcDest = { rect.left, rect.top, rect.right, rect.bottom };
	if (textureTarget_ == NULL) {
		pDevice_->Clear(1, &rcDest, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0, 0);
	} else {
		pDevice_->Clear(1, &rcDest, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0, 0);
	}
}
void DirectGraphics::SetRenderTarget(gstd::ref_count_ptr<Texture> texture)
{
	textureTarget_ = texture;
	if (texture == NULL) {
		pDevice_->SetRenderTarget(0, pBackSurf_);
		pDevice_->SetDepthStencilSurface(pZBuffer_);
	} else {
		pDevice_->SetRenderTarget(0, texture->GetD3DSurface());
		pDevice_->SetDepthStencilSurface(texture->GetD3DZBuffer());
	}
	_InitializeDeviceState();
}
void DirectGraphics::SetLightingEnable(bool bEnable)
{
	pDevice_->SetRenderState(D3DRS_LIGHTING, bEnable);
}
void DirectGraphics::SetSpecularEnable(bool bEnable)
{
	pDevice_->SetRenderState(D3DRS_SPECULARENABLE, bEnable);
}
void DirectGraphics::SetCullingMode(DWORD mode)
{
	pDevice_->SetRenderState(D3DRS_CULLMODE, mode);
}
void DirectGraphics::SetShadingMode(DWORD mode)
{
	pDevice_->SetRenderState(D3DRS_SHADEMODE, mode);
}
void DirectGraphics::SetZBufferEnable(bool bEnable)
{
	pDevice_->SetRenderState(D3DRS_ZENABLE, bEnable);
}
void DirectGraphics::SetZWriteEnalbe(bool bEnable)
{
	pDevice_->SetRenderState(D3DRS_ZWRITEENABLE, bEnable);
}
void DirectGraphics::SetAlphaTest(bool bEnable, DWORD ref, D3DCMPFUNC func)
{
	pDevice_->SetRenderState(D3DRS_ALPHATESTENABLE, bEnable);
	if (bEnable) {
		pDevice_->SetRenderState(D3DRS_ALPHAFUNC, func);
		pDevice_->SetRenderState(D3DRS_ALPHAREF, ref);
	}
}
void DirectGraphics::SetBlendMode(DWORD mode, int stage)
{
	switch (mode) {
	case MODE_BLEND_NONE: //なし
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		break;
	case MODE_BLEND_ALPHA: //αで半透明合成
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case MODE_BLEND_ADD_RGB: //RGBで加算合成
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	case MODE_BLEND_ADD_ARGB: //αで加算合成
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE); //ARG1とARG2のα値を乗算してα値を取得します。
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); //テクスチャのα値
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); //頂点のα値
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); //ARG1とARG2のカラーの値を乗算します。
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE); //テクスチャのカラー
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //頂点のカラー
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	case MODE_BLEND_MULTIPLY: //乗算合成
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
		// pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		// pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		/*
			pDevice_->SetTextureStageState(stage,D3DTSS_COLOROP, D3DTOP_MODULATE);
			pDevice_->SetTextureStageState(stage,D3DTSS_ALPHAARG2, D3DTA_CURRENT);
			pDevice_->SetTextureStageState(stage,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
		*/
		break;
		/*
		case MODE_BLEND_SUBTRACT://減算合成
			pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP,D3DTOP_MODULATE);
			pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2,D3DTA_CURRENT);
			pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		*/
	case MODE_BLEND_SUBTRACT: //減算合成
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE); //ARG1とARG2のα値を乗算してα値を取得します。
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); //テクスチャのα値
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); //頂点のα値
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); //ARG1とARG2のカラーの値を乗算します。
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE); //テクスチャのカラー
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //頂点のカラー
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	case MODE_BLEND_SHADOW: //影描画用
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		break;
	case MODE_BLEND_INV_DESTRGB: //描画先色反転合成
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE); //ARG1とARG2のα値を乗算してα値を取得します。
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); //テクスチャのα値
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); //頂点のα値
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); //ARG1とARG2のカラーの値を乗算します。
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE); //テクスチャのカラー
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //頂点のカラー
		pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
		pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		break;
	}
	// 減算半透明合成
	// pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
	// pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	// pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// ハイライト(覆い焼き)
	// pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	// pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	// pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// リバース(反転)
	// pDevice_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	// pDevice_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
	// pDevice_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}
void DirectGraphics::SetFillMode(DWORD mode)
{
	pDevice_->SetRenderState(D3DRS_FILLMODE, mode);
}
void DirectGraphics::SetFogEnable(bool bEnable)
{
	pDevice_->SetRenderState(D3DRS_FOGENABLE, bEnable ? TRUE : FALSE);
}
bool DirectGraphics::IsFogEnable()
{
	DWORD fog = FALSE;
	pDevice_->GetRenderState(D3DRS_FOGENABLE, &fog);
	bool res = fog == TRUE;
	return res;
}
void DirectGraphics::SetVertexFog(bool bEnable, D3DCOLOR color, float start, float end)
{
	SetFogEnable(bEnable);
	pDevice_->SetRenderState(D3DRS_FOGCOLOR, color);
	pDevice_->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	pDevice_->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&start));
	pDevice_->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&end));
}
void DirectGraphics::SetPixelFog(bool bEnable, D3DCOLOR color, float start, float end)
{
}
void DirectGraphics::SetTextureFilter(DWORD mode, int stage)
{
	switch (mode) {
	case MODE_TEXTURE_FILTER_NONE:
		pDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		pDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
		break;
	case MODE_TEXTURE_FILTER_POINT:
		pDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		break;
	case MODE_TEXTURE_FILTER_LINEAR:
		pDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		break;
	}
}
DWORD DirectGraphics::GetTextureFilter(int stage)
{
	int res = MODE_TEXTURE_FILTER_NONE;
	DWORD mode;
	pDevice_->GetSamplerState(stage, D3DSAMP_MINFILTER, &mode);
	switch (mode) {
	case D3DTEXF_NONE:
		res = MODE_TEXTURE_FILTER_NONE;
		break;
	case D3DTEXF_POINT:
		res = MODE_TEXTURE_FILTER_POINT;
		break;
	case D3DTEXF_LINEAR:
		res = MODE_TEXTURE_FILTER_LINEAR;
		break;
	}
	return res;
}
void DirectGraphics::SetDirectionalLight(D3DVECTOR& dir)
{
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 0.5f;
	light.Diffuse.g = 0.5f;
	light.Diffuse.b = 0.5f;
	light.Ambient.r = 0.5f;
	light.Ambient.g = 0.5f;
	light.Ambient.b = 0.5f;
	light.Direction = dir;
	pDevice_->SetLight(0, &light);
	pDevice_->LightEnable(0, TRUE);
}
void DirectGraphics::SetViewPort(int x, int y, int width, int height)
{
	D3DVIEWPORT9 viewPort;
	ZeroMemory(&viewPort, sizeof(D3DVIEWPORT9));
	viewPort.X = x;
	viewPort.Y = y;
	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.MinZ = 0.0f;
	viewPort.MaxZ = 1.0f;
	pDevice_->SetViewport(&viewPort);
}
void DirectGraphics::ResetViewPort()
{
	SetViewPort(0, 0, GetScreenWidth(), GetScreenHeight());
}
int DirectGraphics::GetScreenWidth()
{
	return config_.GetScreenWidth();
}
int DirectGraphics::GetScreenHeight()
{
	return config_.GetScreenHeight();
}
double DirectGraphics::GetScreenWidthRatio()
{
	int w, h;
	SDL_GetWindowSize(hAttachedWindow_, &w, &h);
	return w / config_.GetScreenWidth();
}
double DirectGraphics::GetScreenHeightRatio()
{
	int w, h;
	SDL_GetWindowSize(hAttachedWindow_, &w, &h);
	return h / config_.GetScreenHeight();
}
POINT DirectGraphics::GetMousePosition()
{
	int x, y;
	SDL_GetMouseState(&x, &y);

	double ratioWidth = GetScreenWidthRatio();
	double ratioHeight = GetScreenHeightRatio();
	if (ratioWidth != 0) {
		x /= ratioWidth;
	}
	if (ratioHeight != 0) {
		y /= ratioHeight;
	}

	return { x, y };
}
void DirectGraphics::SaveBackSurfaceToFile(std::wstring path)
{
	RECT rect = { 0, 0, config_.GetScreenWidth(), config_.GetScreenHeight() };
	LPDIRECT3DSURFACE9 pBackSurface = NULL;
	pDevice_->GetRenderTarget(0, &pBackSurface);
	D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP,
		pBackSurface, NULL, &rect);
	pBackSurface->Release();
}
bool DirectGraphics::IsPixelShaderSupported(int major, int minor)
{
	D3DCAPS9 caps;
	pDevice_->GetDeviceCaps(&caps);
	bool res = caps.PixelShaderVersion >= D3DPS_VERSION(major, minor);
	return res;
}

/**********************************************************
//DirectGraphicsPrimaryWindow
**********************************************************/
DirectGraphicsPrimaryWindow::DirectGraphicsPrimaryWindow()
{
}
DirectGraphicsPrimaryWindow::~DirectGraphicsPrimaryWindow()
{
}
void DirectGraphicsPrimaryWindow::_PauseDrawing()
{
	// TODO: This needs to be changed by DirectX
	//   NOT by the window handle -.-''

	gstd::Application::GetBase()->SetActive(false);
}
void DirectGraphicsPrimaryWindow::_RestartDrawing()
{
	gstd::Application::GetBase()->SetActive(true);
}
bool DirectGraphicsPrimaryWindow::Initialize()
{
	this->Initialize(config_);
	return true;
}
bool DirectGraphicsPrimaryWindow::Initialize(DirectGraphicsConfig& config)
{
	int wWidth = config.GetScreenWidth();
	int wHeight = config.GetScreenHeight();

	// TODO: Should we add OpenGL/Vulkan here?
	// TODO: Would High DPI work on this game?
	int flags = SDL_WINDOW_ALLOW_HIGHDPI;

	if (config.IsFullScreen())
		// TODO: Borderless fullscreen ?
		flags &= SDL_WINDOW_FULLSCREEN;

	hWnd_ = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wWidth, wHeight, flags);

	if (!hWnd_)
		return false;

#if 0
	this->Attach(hWnd_);
#endif

	DirectGraphics::Initialize(hWnd_, config);

	return true;
}

void DirectGraphicsPrimaryWindow::EventProcedure(SDL_Event* evt)
{
	switch (evt->type)
	{
	case SDL_WINDOWEVENT:
		switch (evt->window.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_APP_WILLENTERBACKGROUND: // Android/iOS/WinRT
			_RestartDrawing();
			break;

		case SDL_WINDOWEVENT_HIDDEN:
		case SDL_APP_WILLENTERFOREGROUND: // Android/iOS/WinRT
			_PauseDrawing();
			break;

		case SDL_WINDOWEVENT_CLOSE:
			// TODO: Send shutdown
			break;

		default:
			break;
		}

		break;

	case SDL_APP_TERMINATING: // Android/IOS/WinRT
	case SDL_QUIT:
		// TODO: Send shutdown
		break;

	case SDL_KEYDOWN:
		if (evt->key.keysym.sym == SDLK_F12)
		{
			// TODO: Send shutdown
		}

		if ( (evt->key.keysym.sym == SDLK_RETURN || evt->key.keysym.sym == SDLK_RETURN2)
			&& (evt->key.keysym.mod == KMOD_LALT || evt->key.keysym.mod == KMOD_RALT) )
			ChangeScreenMode();
		break;
	}
}

void DirectGraphicsPrimaryWindow::ChangeScreenMode()
{
	Application::GetBase()->SetActive(true);

	//テクスチャ解放
	_ReleaseDxResource();

	if (modeScreen_ == SCREENMODE_FULLSCREEN) {
		pDevice_->Reset(&d3dppWin_);
		
		SDL_SetWindowFullscreen(hAttachedWindow_, 0);

		SDL_SetWindowPosition(hAttachedWindow_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		modeScreen_ = SCREENMODE_WINDOW;
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		pDevice_->Reset(&d3dppFull_);

		// TODO: borderless fullscreen?
		SDL_SetWindowFullscreen(hAttachedWindow_, SDL_WINDOW_FULLSCREEN);

		SDL_ShowCursor(SDL_DISABLE);
		modeScreen_ = SCREENMODE_FULLSCREEN;
	}

	//テクスチャレストア
	_RestoreDxResource();
}

/**********************************************************
//DxCamera
**********************************************************/
DxCamera::DxCamera()
{
	Reset();
}
DxCamera::~DxCamera()
{
}
void DxCamera::Reset()
{
	radius_ = 500;
	angleAzimuth_ = 15;
	angleElevation_ = 45;
	pos_.x = 0;
	pos_.y = 0;
	pos_.z = 0;

	yaw_ = 0;
	pitch_ = 0;
	roll_ = 0;

	clipNear_ = 10;
	clipFar_ = 2000;
}

D3DXVECTOR3 DxCamera::GetCameraPosition()
{
	D3DXVECTOR3 res;
	res.x = pos_.x + (float)(radius_ * cos(D3DXToRadian(angleElevation_)) * cos(D3DXToRadian(angleAzimuth_)));
	res.y = pos_.y + (float)(radius_ * sin(D3DXToRadian(angleElevation_)));
	res.z = pos_.z + (float)(radius_ * cos(D3DXToRadian(angleElevation_)) * sin(D3DXToRadian(angleAzimuth_)));
	return res;
}
D3DXMATRIX DxCamera::GetMatrixLookAtLH()
{
	D3DXMATRIX res;
	D3DXVECTOR3 posCamera = GetCameraPosition();

	D3DXVECTOR3 vCameraUp(0, 1, 0);
	{
		D3DXQUATERNION qRot(0, 0, 0, 1.0f);
		D3DXQuaternionRotationYawPitchRoll(&qRot,
			Math::DegreeToRadian(yaw_), Math::DegreeToRadian(pitch_), Math::DegreeToRadian(roll_));
		D3DXMATRIX matRot;
		D3DXMatrixRotationQuaternion(&matRot, &qRot);
		D3DXVec3TransformCoord((D3DXVECTOR3*)&vCameraUp, (D3DXVECTOR3*)&vCameraUp, &matRot);
	}

	D3DXVECTOR3 posTo = pos_;
	{
		D3DXMATRIX matTrans1;
		D3DXMatrixTranslation(&matTrans1, -posCamera.x, -posCamera.y, -posCamera.z);
		D3DXMATRIX matTrans2;
		D3DXMatrixTranslation(&matTrans2, posCamera.x, posCamera.y, posCamera.z);

		float pitch = pitch_;

		D3DXQUATERNION qRot(0, 0, 0, 1.0f);
		D3DXQuaternionRotationYawPitchRoll(&qRot,
			Math::DegreeToRadian(yaw_), Math::DegreeToRadian(pitch_), Math::DegreeToRadian(0));
		D3DXMATRIX matRot;
		D3DXMatrixRotationQuaternion(&matRot, &qRot);

		D3DXMATRIX mat;
		mat = matTrans1 * matRot * matTrans2;
		D3DXVec3TransformCoord((D3DXVECTOR3*)&posTo, (D3DXVECTOR3*)&posTo, &mat);
	}

	D3DXMatrixLookAtLH(&res, &posCamera, &posTo, &vCameraUp);
	return res;
}
void DxCamera::UpdateDeviceWorldViewMatrix()
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	if (graph == NULL)
		return;
	IDirect3DDevice9* device = graph->GetDevice();

	D3DXMATRIX matView = GetMatrixLookAtLH();
	device->SetTransform(D3DTS_VIEW, &matView);
}
void DxCamera::SetProjectionMatrix(float width, float height, float posNear, float posFar)
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	if (graph == NULL)
		return;
	IDirect3DDevice9* device = graph->GetDevice();

	D3DXMatrixPerspectiveFovLH(&matProjection_, D3DXToRadian(45.0),
		width / height, posNear, posFar);

	if (clipNear_ < 1)
		clipNear_ = 1;
	if (clipFar_ < 1)
		clipFar_ = 1;
	clipNear_ = posNear;
	clipFar_ = posFar;

	/*
	ref_count_ptr<DxCamera2D> camera2D = graph->GetCamera2D();
	D3DXVECTOR2 pos = camera2D->GetLeftTopPosition();
	double ratio = camera2D->GetRatio();
	D3DXMATRIX matScale;
	D3DXMatrixScaling(&matScale, ratio, ratio, 1.0);
	D3DXMATRIX matTrans;
	D3DXMatrixTranslation(&matTrans, pos.x / width, pos.y / height, 0);

	persMat = persMat * matScale;
	persMat = persMat * matTrans;
	*/
}
void DxCamera::UpdateDeviceProjectionMatrix()
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	if (graph == NULL)
		return;
	IDirect3DDevice9* device = graph->GetDevice();
	device->SetTransform(D3DTS_PROJECTION, &matProjection_);
}
D3DXVECTOR2 DxCamera::TransformCoordinateTo2D(D3DXVECTOR3 pos)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	IDirect3DDevice9* device = graphics->GetDevice();
	int width = graphics->GetConfigData().GetScreenWidth();
	int height = graphics->GetConfigData().GetScreenHeight();

	D3DXMATRIX matView;
	device->GetTransform(D3DTS_VIEW, &matView);
	D3DXMATRIX matPers;
	device->GetTransform(D3DTS_PROJECTION, &matPers);

	D3DXMATRIX mat = matView * matPers;
	D3DXVECTOR4 vect;
	vect.x = pos.x;
	vect.y = pos.y;
	vect.z = pos.z;
	vect.w = 1;

	float vx = vect.x;
	float vy = vect.y;
	float vz = vect.z;

	vect.x = (vx * mat._11) + (vy * mat._21) + (vz * mat._31) + mat._41;
	vect.y = (vx * mat._12) + (vy * mat._22) + (vz * mat._32) + mat._42;
	vect.z = (vx * mat._13) + (vy * mat._23) + (vz * mat._33) + mat._43;
	vect.w = (vx * mat._14) + (vy * mat._24) + (vz * mat._34) + mat._44;

	if (vect.w > 0) {
		vect.x = width / 2 + (vect.x / vect.w) * width / 2;
		vect.y = height / 2 - (vect.y / vect.w) * height / 2; // Ｙ方向は上が正となるため
	}

	D3DXVECTOR2 res(vect.x, vect.y);
	return res;
}

/**********************************************************
//DxCamera2D
**********************************************************/
DxCamera2D::DxCamera2D()
{
	pos_.x = 400;
	pos_.y = 300;
	ratioX_ = 1.0;
	ratioY_ = 1.0;
	angleZ_ = 0;
	bEnable_ = false;
}
DxCamera2D::~DxCamera2D()
{
}
void DxCamera2D::Reset()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int width = graphics->GetScreenWidth();
	int height = graphics->GetScreenHeight();
	if (posReset_ == NULL) {
		pos_.x = width / 2;
		pos_.y = height / 2;
	} else {
		pos_.x = posReset_->x;
		pos_.y = posReset_->y;
	}
	ratioX_ = 1.0;
	ratioY_ = 1.0;
	SetRect(&rcClip_, 0, 0, width, height);

	angleZ_ = 0;
}
D3DXVECTOR2 DxCamera2D::GetLeftTopPosition()
{
	return GetLeftTopPosition(pos_, ratioX_, ratioY_, rcClip_);
}
D3DXVECTOR2 DxCamera2D::GetLeftTopPosition(D3DXVECTOR2 focus, double ratio)
{
	return GetLeftTopPosition(focus, ratio, ratio);
}
D3DXVECTOR2 DxCamera2D::GetLeftTopPosition(D3DXVECTOR2 focus, double ratioX, double ratioY)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int width = graphics->GetScreenWidth();
	int height = graphics->GetScreenHeight();
	RECT rcClip;
	ZeroMemory(&rcClip, sizeof(RECT));
	rcClip.right = width;
	rcClip.bottom = height;
	return GetLeftTopPosition(focus, ratioX, ratioY, rcClip);
}
D3DXVECTOR2 DxCamera2D::GetLeftTopPosition(D3DXVECTOR2 focus, double ratioX, double ratioY, RECT rcClip)
{
	int width = rcClip.right - rcClip.left;
	int height = rcClip.bottom - rcClip.top;

	int cx = rcClip.left + width / 2; //画面の中心座標x
	int cy = rcClip.top + height / 2; //画面の中心座標y

	int dx = focus.x - cx; //現フォーカスでの画面左端位置
	int dy = focus.y - cy; //現フォーカスでの画面上端位置

	D3DXVECTOR2 res;
	res.x = cx - dx * ratioX; //現フォーカスでの画面中心の位置(x座標変換量)
	res.y = cy - dy * ratioY; //現フォーカスでの画面中心の位置(y座標変換量)

	res.x -= (width / 2) * ratioX; //現フォーカスでの画面左の位置(x座標変換量)
	res.y -= (height / 2) * ratioY; //現フォーカスでの画面中心の位置(x座標変換量)

	return res;
}

D3DXMATRIX DxCamera2D::GetMatrix()
{
	D3DXVECTOR2 pos = GetLeftTopPosition();
	D3DXMATRIX matScale;
	D3DXMatrixScaling(&matScale, ratioX_, ratioY_, 1.0);
	D3DXMATRIX matTrans;
	D3DXMatrixTranslation(&matTrans, pos.x, pos.y, 0);

	D3DXMATRIX matAngleZ;
	D3DXMatrixIdentity(&matAngleZ);
	if (angleZ_ != 0) {
		D3DXMATRIX matTransRot1;
		D3DXMatrixTranslation(&matTransRot1, -GetFocusX() + pos.x, -GetFocusY() + pos.y, 0);
		D3DXMATRIX matRot;
		D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, D3DXToRadian(angleZ_));
		D3DXMATRIX matTransRot2;
		D3DXMatrixTranslation(&matTransRot2, GetFocusX() - pos.x, GetFocusY() - pos.y, 0);
		matAngleZ = matTransRot1 * matRot * matTransRot2;
	}

	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	mat = mat * matScale;
	mat = mat * matAngleZ;
	mat = mat * matTrans;
	return mat;
}
