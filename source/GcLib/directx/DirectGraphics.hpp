#ifndef __DIRECTX_DIRECTGRAPHICS__
#define __DIRECTX_DIRECTGRAPHICS__

#include "DxConstant.hpp"

namespace directx {

class DxCamera;
class DxCamera2D;
class FrameBuffer;
struct ShaderData;
	
/**********************************************************
//DirectGraphicsConfig
**********************************************************/
struct DirectGraphicsConfig
{
	enum class ColorMode
	{
		Bit16,
		Bit32,
	};

	enum class ScreenMode
	{
		Fullscreen,
		DesktopFullscreen,
		Windowed,
	};

	DirectGraphicsConfig() : RenderWidth(800), RenderHeight(600), Color(ColorMode::Bit32),
		UseTripleBuffer(false), IsFullscreen(false), UseVSync(false), Render(bgfx::RendererType::Direct3D9) {}

	uint16_t RenderWidth, RenderHeight;
	ColorMode Color;
	bool UseTripleBuffer, IsFullscreen, UseVSync;
	bgfx::RendererType::Enum Render;
};

class DirectGraphicsListener
{
public:
	virtual ~DirectGraphicsListener() = default;
	virtual void ReleaseDirectGraphics() {}
	virtual void RestoreDirectGraphics() {}
	virtual void StartChangeScreenMode() { ReleaseDirectGraphics(); }
	virtual void EndChangeScreenMode() { RestoreDirectGraphics(); }
};

class DirectGraphics
	{
	static DirectGraphics* thisBase_;

public:	
	enum class BlendMode
	{
		None, //なし
		Alpha, //αで半透明合成
		Add_RGB, //RGBで加算合成
		Add_ARGB, //αで加算合成
		Multiply, //乗算合成
		Subtract, //減算合成
		Shadow, //影描画用
		InvDestRGB, //描画先色反転合成
	};

	enum class TextureFilterMode
	{
		None, //フィルタなし
		Point, //補間なし
		//Linear, //線形補間
		Anisotropic,
	};

	enum class CullingMode
	{
		None,
		Cw,
		Ccw,
	};

	enum class ShadeMode
	{
		Flat,
		Gouraud,
		Phong,
	};

	enum class DepthMode
	{
		None,
		Less,
		LessEqual,
		Equal,
		GreaterEqual,
		NotEqual,
		Never,
		Always,
	};

	DirectGraphics();
	virtual ~DirectGraphics();
	static DirectGraphics* GetBase() { return thisBase_; }

	virtual bool Initialize(void* nwh, void* ndt);
	virtual bool Initialize(DirectGraphicsConfig& config, void* nwh, void* ndt);
	void Shutdown();
	
	void AddDirectGraphicsListener(DirectGraphicsListener* listener);
	void RemoveDirectGraphicsListener(DirectGraphicsListener* listener);

	DirectGraphicsConfig& GetConfigData() { return config_; }

	void BeginScene(bool bClear = true); //描画開始
	void EndScene() const; //描画終了
	void Clear(bgfx::ViewId id = 0) const;
	void Clear(bgfx::ViewId id, uint16_t x, uint16_t y, uint16_t width, uint16_t height) const;

	void SetProjMatrix(glm::mat4 mtx, bgfx::ViewId id = 0);
	void SetViewMatrix(glm::mat4 mtx, bgfx::ViewId id = 0);
	glm::mat4 GetProjMatrix() const { return matProj_; }
	glm::mat4 GetViewMatrix() const { return matView_; }
	
	void SetViewAndProjMatrix(glm::mat4 view, glm::mat4 proj, bgfx::ViewId id = 0);

	void SetRenderTarget(std::shared_ptr<FrameBuffer>& texture, bgfx::ViewId id = 0);

	//レンダリングステートラッパ
	void SetSpecularEnable(bool bEnable); //スペキュラ
	void SetCullingMode(CullingMode mode); //カリング
	void SetShadingMode(ShadeMode mode); //シェーディング
	void SetDepthTest(DepthMode mode); //Was ZBuffer before
	void SetZWriteEnable(bool bEnable); //Zバッファ書き込み
	void SetAlphaTest(bool bEnable, DWORD ref = 0);
	void SetBlendMode(BlendMode mode, int stage = 0);
	void SetFogEnable(bool bEnable);
	bool IsFogEnable() const;
	void SetVertexFog(bool bEnable, D3DCOLOR color, float start, float end);
	void SetPixelFog(bool bEnable, D3DCOLOR color, float start, float end);
	void SetTextureFilter(TextureFilterMode mode);
	TextureFilterMode GetTextureFilter(int stage = 0) const;

	void SetDirectionalLight(glm::vec3 v);

	uint16_t GetRenderWidth() const { return config_.RenderWidth; }
	uint16_t GetRenderHeight() const { return config_.RenderHeight; }

	float GetScreenWidthRatio() const;
	float GetScreenHeightRatio() const;

	DxCamera* GetCamera() const { return camera_.get(); }
	DxCamera2D* GetCamera2D() const { return camera2D_.get(); }
		
	void UpdateState() const;

	bgfx::ViewId AddView(std::string name);
	void RemoveView(bgfx::ViewId id);

	void Submit(bgfx::ViewId id, bgfx::ProgramHandle prog);
	void RestoreViews();

	void SetLightingEnable(bool b);

	uint32_t GetSamplerFlags() const { return samplerFlags_; }

protected:

	DirectGraphicsConfig config_;
	std::list<DirectGraphicsListener*> listListener_;

	std::unique_ptr<DxCamera> camera_;
	std::unique_ptr<DxCamera2D> camera2D_;
	std::unordered_map<uint16_t, std::shared_ptr<FrameBuffer>> textureTarget_;
	std::shared_ptr<FrameBuffer> defaultFB_;

	void _ReleaseDxResource();
	void _RestoreDxResource();
	void _Restore();
	void _InitializeDeviceState();

	uint64_t states_;
	uint32_t resetFlags_;
	uint32_t blendFactor_;
	uint16_t clearFlags_;
	bool init_;

	glm::mat4 matProj_, matView_;

	std::vector<bgfx::ViewId> views_;

	// Work for the shader !
	
	std::shared_ptr<ShaderData> shader_;
	glm::vec4 sh_options_;
	bgfx::UniformHandle uniforms_[6];
	glm::vec4 sh_dirlight_diffuse, sh_dirlight_ambient, sh_dirlight_direction;
	glm::vec4 sh_amblight;
	uint32_t samplerFlags_;
};

// TODO: Migrate to an application specific part
#if 0
/**********************************************************
//DirectGraphicsPrimaryWindow
**********************************************************/
class DirectGraphicsPrimaryWindow : public DirectGraphics, public gstd::WindowBase {
public:
	DirectGraphicsPrimaryWindow();
	~DirectGraphicsPrimaryWindow();
	virtual bool Initialize();
	virtual bool Initialize(DirectGraphicsConfig& config);
	void ChangeScreenMode();

protected:
	virtual void EventProcedure(SDL_Event* evt); //オーバーライド用プロシージャ
	void _PauseDrawing();
	void _RestartDrawing();
};
#endif
	
/**********************************************************
//DxCamera
**********************************************************/
class DxCamera
	{
public:
	DxCamera();
	virtual ~DxCamera() = default;
	void Reset();
	
	glm::vec3 GetCameraPosition() const;
	glm::vec3 GetFocusPosition() const { return pos_; }
	
	void SetFocus(float x, float y, float z)
	{
		pos_.x = x;
		pos_.y = y;
		pos_.z = z;
	}
	void SetFocus(glm::vec3 f) { pos_ = f; }
	void SetFocusX(const float x) { pos_.x = x; }
	void SetFocusY(const float y) { pos_.y = y; }
	void SetFocusZ(const float z) { pos_.z = z; }
	
	float GetRadius() const  { return radius_; }
	void SetRadius(const float r) { radius_ = r; }
	float GetAzimuthAngle() const  { return angleAzimuth_; }
	void SetAzimuthAngle(const float angle) { angleAzimuth_ = angle; }
	float GetElevationAngle() const  { return angleElevation_; }
	void SetElevationAngle(const float angle) { angleElevation_ = angle; }

	float GetYaw() const { return yaw_; }
	void SetYaw(const float yaw) { yaw_ = yaw; }
	float GetPitch() const  { return pitch_; }
	void SetPitch(const float pitch) { pitch_ = pitch; }
	float GetRoll() const  { return roll_; }
	void SetRoll(const float roll) { roll_ = roll; }

	float GetNearClip() const  { return clipNear_; }
	float GetFarClip() const  { return clipFar_; }

	glm::mat4 GetMatrixLookAtLH() const;
	void UpdateDeviceProjectionMatrix(bgfx::ViewId id = 0) const;
	void UpdateDeviceWorldViewMatrix(bgfx::ViewId id = 0) const;
	void SetProjectionMatrix(float width, float height, float posNear, float posFar, float fov = 45.0f);

	glm::vec2 TransformCoordinateTo2D(glm::vec3 pos);

private:
	glm::vec3 pos_; //焦点
	float radius_;
	float angleAzimuth_;
	float angleElevation_;
	glm::mat4 matProjection_;

	float yaw_;
	float pitch_;
	float roll_;

	float clipNear_;
	float clipFar_;
};

/**********************************************************
//DxCamera2D
**********************************************************/
class DxCamera2D
{
public:
	DxCamera2D();
	virtual ~DxCamera2D() = default;

	bool IsEnable() const { return bEnable_; }
	void SetEnable(const bool bEnable) { bEnable_ = bEnable; }

	glm::vec2 GetFocusPosition() const { return pos_; }
	float GetFocusX() const { return pos_.x; }
	float GetFocusY() const { return pos_.y; }
	void SetFocus(const float x, const float y)
	{
		pos_.x = x;
		pos_.y = y;
	}
	void SetFocus(const glm::vec2 pos) { pos_ = pos; }
	void SetFocusX(const float x) { pos_.x = x; }
	void SetFocusY(const float y) { pos_.y = y; }
	float GetRatio() const { return glm::min(ratioX_, ratioY_); }
	void SetRatio(const float ratio)
	{
		ratioX_ = ratio;
		ratioY_ = ratio;
	}
	float GetRatioX() const { return ratioX_; }
	void SetRatioX(const float ratio) { ratioX_ = ratio; }
	float GetRatioY() const { return ratioY_; }
	void SetRatioY(const float ratio) { ratioY_ = ratio; }
	float GetAngleZ() const { return angleZ_; }
	void SetAngleZ(const float angle) { angleZ_ = angle; }

	RECT GetClip() const { return rcClip_; }
	void SetClip(const RECT rect) { rcClip_ = rect; }

	void SetResetFocus(gstd::ref_count_ptr<glm::vec2> pos) { posReset_ = pos; }
	void Reset();
	inline glm::vec2 GetLeftTopPosition() const;
	inline static glm::vec2 GetLeftTopPosition(glm::vec2 focus, float ratio);
	inline static glm::vec2 GetLeftTopPosition(glm::vec2 focus, float ratioX, float ratioY);
	inline static glm::vec2 GetLeftTopPosition(glm::vec2 focus, float ratioX, float ratioY, RECT rcClip);

	glm::mat4 GetMatrix() const;

private:
	bool bEnable_;
	glm::vec2 pos_; //焦点
	float ratioX_; //拡大率
	float ratioY_;
	float angleZ_;
	RECT rcClip_; //視野

	gstd::ref_count_ptr<glm::vec2> posReset_;
};

} // namespace directx

#endif
