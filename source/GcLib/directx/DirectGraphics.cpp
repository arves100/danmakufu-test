#include "DirectGraphics.hpp"
#include "DxUtility.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

using namespace gstd;
using namespace directx;

/* Shader options x */
#define SOX_LIGHT 1 << 1
#define SOX_FOG 1 << 2
#define SOX_SPECULAR 1 << 3

/* Shader options y */
#define SOY_SHADEMODE_FLAT 1 << 0
#define SOY_SHADEMODE_GOURAUD 1 << 1
#define SOY_SHADEMODE_PHONG 1 << 2
#define SOY_SHADEMODE_FLAG 7

/**********************************************************
//DirectGraphics
**********************************************************/
DirectGraphics* DirectGraphics::thisBase_ = nullptr;

//192, 192, 192

DirectGraphics::DirectGraphics() : states_(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A), resetFlags_(0), blendFactor_(0),
	clearFlags_(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH), init_(false), matProj_(), matView_(), sh_options_(0.0f, 0.0f, 0.0f, 0.0f),
	sh_dirlight_direction(0.0f), sh_dirlight_ambient(0.5f, 0.5f, 0.5f, 1.0f), sh_dirlight_diffuse(0.5f, 0.5f, 0.5f, 1.0f),
	sh_amblight(0.75f, 0.75f, 0.75f, 1.0f)
{
	camera_ = std::make_unique<DxCamera>();
	camera2D_ = std::make_unique<DxCamera2D>();

	uniforms_[0] = BGFX_INVALID_HANDLE;
	uniforms_[1] = BGFX_INVALID_HANDLE;
	uniforms_[2] = BGFX_INVALID_HANDLE;
	uniforms_[3] = BGFX_INVALID_HANDLE;
	uniforms_[4] = BGFX_INVALID_HANDLE;
}
DirectGraphics::~DirectGraphics()
{
	Shutdown();
	thisBase_ = nullptr;
}

bool DirectGraphics::Initialize(void* nwh, void* ndt)
{
	return Initialize(config_, nwh, ndt);
}

void DirectGraphics::Shutdown()
{
	if (bgfx::isValid(uniforms_[5]))
		bgfx::destroy(uniforms_[5]);
	
	if (bgfx::isValid(uniforms_[4]))
		bgfx::destroy(uniforms_[4]);
	
	if (bgfx::isValid(uniforms_[3]))
		bgfx::destroy(uniforms_[3]);

	if (bgfx::isValid(uniforms_[2]))
		bgfx::destroy(uniforms_[2]);
	
	if (bgfx::isValid(uniforms_[1]))
		bgfx::destroy(uniforms_[1]);

	if (bgfx::isValid(uniforms_[0]))
		bgfx::destroy(uniforms_[0]);

	shader_.reset();
	defaultFB_.reset();
	
	if (init_)
	{
		bgfx::shutdown();
		Logger::WriteTop(L"DirectGraphic: Bgfx shutdown");
		init_ = false;
	}
}

bool DirectGraphics::Initialize(DirectGraphicsConfig& config, void* nwh, void* ndt)
{
	if (thisBase_ != nullptr)
		return false;

	Logger::WriteTop(L"DirectGraphics: Bgfx creation");

	config_ = config;

	bgfx::Init init;

#ifdef _DEBUG
	init.debug = true;
	init.profile = true;
#else
	init.debug = false;
	init.profile = false;
#endif

	init.resolution.width = config_.RenderWidth;
	init.resolution.height = config_.RenderHeight;
	
	if (config_.Color == DirectGraphicsConfig::ColorMode::Bit16)
		init.resolution.format = bgfx::TextureFormat::R5G6B5;
	else
		init.resolution.format = bgfx::TextureFormat::RGBA8;

	if (!config_.UseTripleBuffer)
		init.resolution.numBackBuffers = 1;
	else
		init.resolution.numBackBuffers = 2;

	init.platformData.nwh = nwh;
	init.platformData.ndt = ndt;

	resetFlags_ = 0;

	// TODO: MSAA (MultiSample = None in dhn)

	if (config_.IsFullscreen)
		resetFlags_ &= BGFX_RESET_FULLSCREEN;

	if (config_.UseVSync)
		resetFlags_ &= BGFX_RESET_VSYNC;
	
	init.resolution.reset = resetFlags_;
	init.type = config.Render;

	init.allocator = DxAllocator::Get();

	/*if (config_.IsWaitTimerEnable() == false)
		d3dppFull_.FullScreen_RefreshRateInHz = 60; // TODO: Is this VSync?
	d3dppFull_.EnableAutoDepthStencil = TRUE; // TODO!
	d3dppFull_.AutoDepthStencilFormat = D3DFMT_D16;*/

	// TODO: Bgfx DOES NOT HAVE ANY ADAPTER DETECTION CODE LIKE DHN DIRECTX9 USED TO

	if (!bgfx::init(init))
	{
		Logger::WriteTop(L"DirectGraphics: Cannot initialize bgfx");
		return false;
	}

#ifdef _DEBUG
	bgfx::setDebug(BGFX_DEBUG_TEXT);
#endif

	init_ = true;
	thisBase_ = this;

	if (camera2D_ != nullptr)
		camera2D_->Reset();
	
	_InitializeDeviceState();

	bgfx::frame();

	shader_ = std::make_shared<ShaderData>();

	uniforms_[0] = bgfx::createUniform("s_viewtex", bgfx::UniformType::Sampler);
	uniforms_[1] = bgfx::createUniform("u_options", bgfx::UniformType::Vec4);
	uniforms_[2] = bgfx::createUniform("u_dl_ambient", bgfx::UniformType::Vec4);
	uniforms_[3] = bgfx::createUniform("u_dl_diffuse", bgfx::UniformType::Vec4);
	uniforms_[4] = bgfx::createUniform("u_dl_direction", bgfx::UniformType::Vec4);
	uniforms_[5] = bgfx::createUniform("u_al", bgfx::UniformType::Vec4);

	Logger::WriteTop(L"DirectGraphics：初期化完了");
	return true;
}

void DirectGraphics::RestoreViews()
{
	views_.clear();
	
#if 0 // TODO: No render shader for now
	// Drawing will work like this
	// We draw everything to view 0 (connected to the default fb)
	// We compute DirectGraphic specific shader work in view1 with the data of view0
	TextureManager::GetBase()->GetFrameBuffer(TextureManager::DEFAULT_FRAMEBUFFER, defaultFB_);
	textureTarget_[0] = defaultFB_;
	bgfx::setViewFrameBuffer(0, defaultFB_->GetHandle());

	// Setup default views
	views_.push_back(1);
	views_.push_back(0);
	bgfx::setViewName(0, "Draw view");
	bgfx::setViewName(1, "Computed view");
	
	Clear(0);
	Clear(1);

	bgfx::setViewOrder(0, static_cast<uint16_t>(views_.size()), &views_[0]);
#else
	views_.push_back(0);
	bgfx::setViewName(0, "Draw view");
	Clear(0);
#endif
}

void DirectGraphics::_ReleaseDxResource()
{
	
	for (auto& itr : listListener_)
		itr->ReleaseDirectGraphics();
}

void DirectGraphics::_RestoreDxResource()
{
	// we can't access the backbuffer on bgfx
	for (auto& itr : listListener_)
		itr->RestoreDirectGraphics();

	_InitializeDeviceState();
}

void DirectGraphics::Submit(bgfx::ViewId id, bgfx::ProgramHandle prog)
{
	if (id == 1 || id + 1 > views_.size())
		return; // you cannot submit meshes to the shader compute view (unless you are the compute shader)

	bgfx::submit(id, prog);
}

void DirectGraphics::_Restore()
{
	Logger::WriteTop(L"DirectGraphics：_Restore開始");

	// リストア
	_ReleaseDxResource();

	//デバイスリセット
	if (config_.IsFullscreen)
		resetFlags_ |= BGFX_RESET_FULLSCREEN; // NOTE: this should not be supported on bgfx
	else
		resetFlags_ &= ~BGFX_RESET_FULLSCREEN;

	if (config_.UseVSync)
		resetFlags_ |= BGFX_RESET_VSYNC;
	else
		resetFlags_ &= ~BGFX_RESET_VSYNC;
	
	bgfx::reset(config_.RenderWidth, config_.RenderHeight, resetFlags_);

	_RestoreDxResource();

	for (auto& tt : textureTarget_)
	{
		bgfx::setViewFrameBuffer(tt.first, tt.second->GetHandle());		
	}

	Logger::WriteTop(L"DirectGraphics：_Restore完了");
}

void DirectGraphics::_InitializeDeviceState()
{
	SetCullingMode(CullingMode::None);

	SetDirectionalLight(glm::vec3(-1.0f, -1.0f, -1.0f));

	SetBlendMode(BlendMode::Alpha); // TODO: might get migrated into something else (like RenderObject)

	SetShadingMode(ShadeMode::Gouraud);

	//αテスト
	SetAlphaTest(true, 0);

	//Filter
	SetTextureFilter(TextureFilterMode::Linear);

	//Zテスト
	SetZWriteEnable(false);
	SetDepthTest(DepthMode::None);

	UpdateState();
}

bgfx::ViewId DirectGraphics::AddView(std::string name)
{
	if (views_.size() >= 65534)
		return 65535; // INVALID!
	
	const bgfx::ViewId id = static_cast<bgfx::ViewId>(views_.size());
	bgfx::setViewName(id, name.c_str());

	auto it = views_.begin();
	std::advance(it, id - 1); // never override the final view!
	views_.insert(it, id);
	bgfx::setViewOrder(0, static_cast<uint16_t>(views_.size()), &views_[0]);
	return id;
}

void DirectGraphics::RemoveView(bgfx::ViewId id)
{
	if (id >= views_.size() || id == 0) // you can not override the default views
		return;

	auto it = views_.begin();
	std::advance(it, id);
	views_.erase(it);
}

void DirectGraphics::AddDirectGraphicsListener(DirectGraphicsListener* listener)
{
	if (std::find(listListener_.begin(), listListener_.end(), listener) == listListener_.end())
		listListener_.push_back(listener);
}

void DirectGraphics::RemoveDirectGraphicsListener(DirectGraphicsListener* listener)
{
	const auto itr = std::find(listListener_.begin(), listListener_.end(), listener);
	if (itr != listListener_.end())
		listListener_.erase(itr);
}

// Bgfx does not allow us to get the view and projection matrix, so we store it in our application
void DirectGraphics::SetViewAndProjMatrix(const glm::mat4 view, const glm::mat4 proj, bgfx::ViewId id)
{
	matView_ = view;
	matProj_ = proj;
	bgfx::setViewTransform(id, &matView_[0], &matProj_[0]);
}

void DirectGraphics::SetProjMatrix(const glm::mat4 mtx, bgfx::ViewId id)
{
	SetViewAndProjMatrix(matView_, mtx, id);
}

void DirectGraphics::SetViewMatrix(const glm::mat4 mtx, bgfx::ViewId id)
{
	SetViewAndProjMatrix(mtx, matProj_, id);
}

void DirectGraphics::BeginScene(bool bClear)
{
	if (bClear)
		Clear();

#ifdef _DEBUG
	bgfx::dbgTextClear();
#endif

	camera_->UpdateDeviceWorldViewMatrix();
	states_ = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
}

void DirectGraphics::EndScene() const
{
#if 0 // TODO
	// Perform view1 drawing
	{
		const auto index = views_[1];
		const auto it = textureTarget_.find(index); // eh???? textureTarget_[index] ???
		const auto& fb = it->second;
		
		bgfx::setTexture(0, uniforms_[0], bgfx::getTexture(fb->GetHandle())); // s_viewtex
		bgfx::setUniform(uniforms_[1], &sh_options_[0]); // u_options
		bgfx::setUniform(uniforms_[2], &sh_dirlight_ambient[0]); // u_dl_ambient
		bgfx::setUniform(uniforms_[3], &sh_dirlight_diffuse[0]); // u_dl_diffuse
		bgfx::setUniform(uniforms_[4], &sh_dirlight_direction[0]); // u_dl_direction
		bgfx::setUniform(uniforms_[5], &sh_amblight[0]); // u_al

		bgfx::submit(1, shader_->Program);
	}
#endif

	bgfx::frame();
}

void DirectGraphics::Clear(bgfx::ViewId id) const
{
	if (id + 1 > views_.size())
		return;
	
	bgfx::setViewClear(id, clearFlags_, 0x00000000, 1.0f, 0);	
	bgfx::setViewRect(id, 0, 0, config_.RenderWidth, config_.RenderHeight);
	bgfx::touch(id);
}

void DirectGraphics::Clear(const bgfx::ViewId id, const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height) const
{
	if (id + 1 > views_.size())
		return;
	
	bgfx::setViewClear(id, clearFlags_, 0x00000000, 1.0f);
	bgfx::setViewRect(id, x, y, width, height);
}

void DirectGraphics::UpdateState() const
{
	bgfx::setState(states_, blendFactor_);
}

void DirectGraphics::SetRenderTarget(std::shared_ptr<FrameBuffer>& texture, bgfx::ViewId id)
{
	if (id + 1 > views_.size())
		return;
	
	textureTarget_[id] = texture;
	if (texture == nullptr)
	{
		bgfx::setViewFrameBuffer(id, BGFX_INVALID_HANDLE);
	}
	else
	{
		bgfx::setViewFrameBuffer(id, textureTarget_[id]->GetHandle());
	}
	
	_InitializeDeviceState();
}

void DirectGraphics::SetSpecularEnable(bool bEnable)
{
	auto option = static_cast<uint32_t>(sh_options_.x);

	if (bEnable)
		option |= SOX_SPECULAR;
	else
		option &= ~SOX_SPECULAR;

	sh_options_.x = static_cast<float>(option);
}

void DirectGraphics::SetCullingMode(CullingMode mode)
{
	// removes culling
	states_ &= ~BGFX_STATE_CULL_MASK;
	
	switch (mode)
	{
	case CullingMode::Cw:
		states_ |= BGFX_STATE_CULL_CW;
		break;
	case CullingMode::Ccw:
		states_ |= BGFX_STATE_CULL_CCW;
		break;
	case CullingMode::None:
		break;
	}
}

void DirectGraphics::SetShadingMode(ShadeMode mode)
{
	auto m = static_cast<int32_t>(sh_options_.y);

	m &= ~SOY_SHADEMODE_FLAG;

	switch (mode)
	{
	case ShadeMode::Flat:
		m |= SOY_SHADEMODE_FLAT;
		break;
	case ShadeMode::Gouraud:
		m |= SOY_SHADEMODE_GOURAUD;
		break;
	case ShadeMode::Phong:
		m |= SOY_SHADEMODE_PHONG;
		break;
	}

	sh_options_.y = static_cast<float>(m);
}

void DirectGraphics::SetDepthTest(DepthMode mode)
{
	states_ &= ~BGFX_STATE_DEPTH_TEST_MASK;

	switch (mode)
	{
	case DepthMode::Always:
		states_ |= BGFX_STATE_DEPTH_TEST_ALWAYS;
		break;
	case DepthMode::Equal:
		states_ |= BGFX_STATE_DEPTH_TEST_EQUAL;
		break;
	case DepthMode::Less:
		states_ |= BGFX_STATE_DEPTH_TEST_LESS;
		break;
	case DepthMode::GreaterEqual:
		states_ |= BGFX_STATE_DEPTH_TEST_GEQUAL;
		break;
	case DepthMode::LessEqual:
		states_ |= BGFX_STATE_DEPTH_TEST_LEQUAL;
		break;
	case DepthMode::Never:
		states_ |= BGFX_STATE_DEPTH_TEST_NEVER;
		break;
	case DepthMode::NotEqual:
		states_ |= BGFX_STATE_DEPTH_TEST_NOTEQUAL;
		break;
	default:
		break;
	}
}

void DirectGraphics::SetZWriteEnable(bool bEnable)
{
	if (bEnable)
		states_ |= BGFX_STATE_WRITE_Z;
	else
		states_ &= ~BGFX_STATE_WRITE_Z;
}

void DirectGraphics::SetAlphaTest(bool bEnable, DWORD ref)
{
	// TODO: DEPRECATED!!! Should be replaced in a frag shader
	
	/*pDevice_->SetRenderState(D3DRS_ALPHATESTENABLE, bEnable);
	if (bEnable) {
		pDevice_->SetRenderState(D3DRS_ALPHAFUNC, func);
		pDevice_->SetRenderState(D3DRS_ALPHAREF, ref);
	}*/
}
void DirectGraphics::SetBlendMode(BlendMode mode, int stage)
{
	states_ &= ~BGFX_STATE_BLEND_MASK; // Clear all blend modes currently set in the state
	
	switch (mode) {
	case BlendMode::None: //なし
		/*pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); // TODO
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);*/
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
		break;
	case BlendMode::Alpha: //αで半透明合成
		/*pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); // TODO
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);*/
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA); // TODO: Is AlphaBlend enabled?
		break;
	case BlendMode::Add_RGB: //RGBで加算合成
		/*pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); // TODO
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);*/
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);  // TODO: Is AlphaBlend enabled?
		break;
	case BlendMode::Add_ARGB: //αで加算合成
		/*pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE); //ARG1とARG2のα値を乗算してα値を取得します。
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); //テクスチャのα値
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); //頂点のα値
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); //ARG1とARG2のカラーの値を乗算します。
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE); //テクスチャのカラー
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //頂点のカラー*/ // TODO
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE); // TODO: Is AlphaBlend enabled?
		break;
	case BlendMode::Multiply: //乗算合成
		/*pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);*/ // TODO
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_SRC_COLOR); // TODO: Is AlphaBlend enabled?
		break;
	case BlendMode::Subtract: //減算合成
		/*pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE); //ARG1とARG2のα値を乗算してα値を取得します。
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); //テクスチャのα値
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); //頂点のα値
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); //ARG1とARG2のカラーの値を乗算します。
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE); //テクスチャのカラー
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //頂点のカラー*/ // TODO
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB); // TODO: Is AlphaBlend enabled?
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		break;
	case BlendMode::Shadow: //影描画用
		/*pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);*/ // TODO
		
		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD); // TODO: Is AlphaBlend enabled?
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_INV_SRC_COLOR);
		break;
	case BlendMode::InvDestRGB: //描画先色反転合成
		/*pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE); //ARG1とARG2のα値を乗算してα値を取得します。
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); //テクスチャのα値
		pDevice_->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); //頂点のα値
		pDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE); //ARG1とARG2のカラーの値を乗算します。
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE); //テクスチャのカラー
		pDevice_->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); //頂点のカラー*/ // TODO

		states_ |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD); // TODO: Is AlphaBlend enabled?
		states_ |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_INV_DST_COLOR, BGFX_STATE_BLEND_INV_SRC_COLOR);
		break;
	}
}

void DirectGraphics::SetFogEnable(bool bEnable)
{
	auto option = static_cast<uint32_t>(sh_options_.x);

	if (bEnable)
		option |= SOX_FOG;
	else
		option &= ~SOX_FOG;

	sh_options_.x = static_cast<float>(option);
}

bool DirectGraphics::IsFogEnable() const
{
	return static_cast<uint32_t>(sh_options_.x) & SOX_FOG;
}

void DirectGraphics::SetVertexFog(bool bEnable, D3DCOLOR color, float start, float end)
{
	/*
	// TODO: DEPRECATED!! Please move to a shader
	SetFogEnable(bEnable);
	pDevice_->SetRenderState(D3DRS_FOGCOLOR, color);
	pDevice_->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	pDevice_->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&start));
	pDevice_->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&end));
	*/
}

void DirectGraphics::SetPixelFog(bool bEnable, D3DCOLOR color, float start, float end)
{
}

void DirectGraphics::SetTextureFilter(TextureFilterMode mode, int stage)
{
	/*
	// TODO: Linear sampling
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
	}*/
}

DirectGraphics::TextureFilterMode DirectGraphics::GetTextureFilter(int stage) const
{
	/*
	// TODO: Linear sampling
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
	return res;*/

	return TextureFilterMode::None; // TODO
}

void DirectGraphics::SetDirectionalLight(glm::vec3 v)
{
	sh_dirlight_direction = glm::vec4(v, 0.0f);
}

float DirectGraphics::GetScreenWidthRatio() const
{
	return 1.0f; // TODO
}

float DirectGraphics::GetScreenHeightRatio() const
{
	return 1.0f; // TODO
}

// TODO: This should be migrated into DhnExecutor
#if 0 
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
	const auto wWidth = config.ScreenWidth, wHeight = config.ScreenHeight;

	// TODO: Should we add OpenGL/Vulkan here?
	// TODO: HighDPI support
	Uint32 flags = 0;

	if (config.Screen == DirectGraphicsConfig::ScreenMode::Fullscreen)
		flags &= SDL_WINDOW_FULLSCREEN;
	else if (config.Screen == DirectGraphicsConfig::ScreenMode::DesktopFullscreen)
		flags &= SDL_WINDOW_FULLSCREEN_DESKTOP;

	hWnd_ = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wWidth, wHeight, flags);

	if (!hWnd_)
		return false;

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
			bShutdown_ = true;
			break;

		default:
			break;
		}

		break;

	case SDL_APP_TERMINATING: // Android/iOS/WinRT
	case SDL_QUIT:
		bShutdown_ = true;
		break;

	case SDL_KEYDOWN:
		if ( (evt->key.keysym.sym == SDLK_RETURN || evt->key.keysym.sym == SDLK_RETURN2)
			&& (evt->key.keysym.mod & KMOD_LALT || evt->key.keysym.mod & KMOD_RALT) )
			ChangeScreenMode();
		break;

	default:
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
#endif

/**********************************************************
//DxCamera
**********************************************************/
DxCamera::DxCamera() : matProjection_()
{
	Reset();
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

glm::vec3 DxCamera::GetCameraPosition() const
{
	glm::vec3 res;
	res.x = pos_.x + (radius_ * glm::cos(glm::radians(angleElevation_)) * glm::cos(glm::radians(angleAzimuth_)));
	res.y = pos_.y + (radius_ * glm::sin(glm::radians(angleElevation_)));
	res.z = pos_.z + (radius_ * glm::cos(glm::radians(angleElevation_)) * glm::sin(glm::radians(angleAzimuth_)));
	return res;
}

glm::mat4 DxCamera::GetMatrixLookAtLH() const
{
	auto posCamera = GetCameraPosition();
	glm::vec4 vCameraUp(0.0f, 1.0f, 0.0f, 0.0f);

	{ // ###E

		const auto matRot = glm::yawPitchRoll(glm::radians(yaw_), glm::radians(pitch_), glm::radians(roll_));
		vCameraUp = matRot * vCameraUp;
	}

	auto posTo = glm::vec4(pos_, 0.0f);
	{
		glm::mat4 matTrans1(1.0f), matTrans2(1.0f);

		matTrans1 = glm::translate(matTrans1, glm::vec3(-posCamera.x, -posCamera.y, -posCamera.z));
		matTrans2 = glm::translate(matTrans2, glm::vec3(posCamera.x, posCamera.y, posCamera.z));

		const auto matRot = glm::yawPitchRoll(glm::radians(yaw_), glm::radians(pitch_), glm::radians(0.0f));
		const auto mat = matTrans1 * matRot * matTrans2;
		posTo = mat * posTo;
	}

	return glm::lookAtLH(posCamera, glm::vec3(posTo), glm::vec3(vCameraUp));
}

void DxCamera::UpdateDeviceWorldViewMatrix(bgfx::ViewId id) const
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	if (!graph)
		return;
	
	const auto mtx = GetMatrixLookAtLH();
	graph->SetViewAndProjMatrix(mtx, matProjection_, id);
}

void DxCamera::UpdateDeviceProjectionMatrix(bgfx::ViewId id) const
{
	DirectGraphics* graph = DirectGraphics::GetBase();
	if (!graph)
		return;

	graph->SetProjMatrix(matProjection_, id);
}

void DxCamera::SetProjectionMatrix(float width, float height, float posNear, float posFar, float fov)
{
	matProjection_ = glm::perspectiveFovLH(glm::radians(fov), width, height, posNear, posFar);
	
	if (clipNear_ < 1.0f)
		clipNear_ = 1.0f;
	if (clipFar_ < 1.0f)
		clipFar_ = 1.0f;
	
	clipNear_ = posNear;
	clipFar_ = posFar;
}

glm::vec2 DxCamera::TransformCoordinateTo2D(glm::vec3 pos) // ###E2
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	const auto width = graphics->GetRenderWidth(), height = graphics->GetRenderHeight();

	const glm::mat4 mat = graphics->GetViewMatrix() * graphics->GetProjMatrix();
	
	glm::vec4 vect = { pos.x, pos.y, pos.z, 1.0f };

	vect = vect * mat;

	if (vect.w > 0) {
		vect.x = static_cast<float>(width) / 2 + (vect.x / vect.w) * static_cast<float>(width) / 2;
		vect.y = static_cast<float>(height) / 2 - (vect.y / vect.w) * static_cast<float>(height) / 2; // Ｙ方向は上が正となるため
	}
	
	return { vect.x , vect.y };
}

/**********************************************************
//DxCamera2D
**********************************************************/
DxCamera2D::DxCamera2D()
{
	pos_.x = 400;
	pos_.y = 300;
	ratioX_ = 1.0f;
	ratioY_ = 1.0f;
	angleZ_ = 0;
	bEnable_ = false;
	rcClip_ = { 0, 0, 0, 0 };
	
}

void DxCamera2D::Reset()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	const auto width = graphics->GetRenderWidth(), height = graphics->GetRenderHeight();
	if (posReset_ == nullptr) {
		pos_.x = static_cast<float>(width) / 2;
		pos_.y = static_cast<float>(height) / 2;
	} else {
		pos_.x = posReset_->x;
		pos_.y = posReset_->y;
	}
	ratioX_ = 1.0f;
	ratioY_ = 1.0f;
	SetRect(&rcClip_, 0, 0, width, height);

	angleZ_ = 0;
}

glm::vec2 DxCamera2D::GetLeftTopPosition() const
{
	return GetLeftTopPosition(pos_, ratioX_, ratioY_, rcClip_);
}

glm::vec2 DxCamera2D::GetLeftTopPosition(glm::vec2 focus, float ratio)
{
	return GetLeftTopPosition(focus, ratio, ratio);
}

glm::vec2 DxCamera2D::GetLeftTopPosition(glm::vec2 focus, float ratioX, float ratioY)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	const auto width = graphics->GetRenderWidth(), height = graphics->GetRenderHeight();
	RECT rcClip;
	memset(&rcClip, 0, sizeof(rcClip));
	rcClip.right = width;
	rcClip.bottom = height;
	return GetLeftTopPosition(focus, ratioX, ratioY, rcClip);
}

glm::vec2 DxCamera2D::GetLeftTopPosition(glm::vec2 focus, float ratioX, float ratioY, RECT rcClip)
{
	const int width = rcClip.right - rcClip.left, height = rcClip.bottom - rcClip.top;

	const int cx = rcClip.left + width / 2; //画面の中心座標x
	const int cy = rcClip.top + height / 2; //画面の中心座標y

	const auto dx = focus.x - cx; //現フォーカスでの画面左端位置
	const auto dy = focus.y - cy; //現フォーカスでの画面上端位置

	glm::vec2 res;
	res.x = static_cast<float>(cx) - dx * ratioX; //現フォーカスでの画面中心の位置(x座標変換量)
	res.y = static_cast<float>(cy) - dy * ratioY; //現フォーカスでの画面中心の位置(y座標変換量)

	res.x -= static_cast<float>(width) / 2 * ratioX; //現フォーカスでの画面左の位置(x座標変換量)
	res.y -= static_cast<float>(height) / 2 * ratioY; //現フォーカスでの画面中心の位置(x座標変換量)

	return res;
}

glm::mat4 DxCamera2D::GetMatrix() const
{
	const auto pos = GetLeftTopPosition();
	glm::mat4 matScale;
	glm::scale(matScale, glm::vec3(ratioX_, ratioY_, 1.0f));
	glm::mat4 matTrans;
	glm::translate(matTrans, glm::vec3(pos.x, pos.y, 0.0f));

	glm::mat4 matAngleZ = glm::identity<glm::mat4>();

	if (static_cast<int>(angleZ_) != 0) {
		glm::mat4 matTransRot1;
		glm::translate(matTransRot1, glm::vec3(-GetFocusX() + pos.x, -GetFocusY() + pos.y, 0.0f));
		const auto matRot = glm::eulerAngleZ(glm::radians(angleZ_));
		glm::mat4 matTransRot2;
		glm::translate(matTransRot2, glm::vec3(GetFocusX() - pos.x, GetFocusY() - pos.y, 0.0f));
		matAngleZ = matTransRot1 * matRot * matTransRot2;
	}

	auto mat = glm::identity<glm::mat4>();
	mat = mat * matScale;
	mat = mat * matAngleZ;
	mat = mat * matTrans;
	return mat;
}

void DirectGraphics::SetLightingEnable(bool b)
{
	auto m = static_cast<uint32_t>(sh_options_.x);

	if (b)
		m |= SOX_LIGHT;
	else
		m &= ~SOX_LIGHT;

	sh_options_.x = static_cast<float>(m);
}
