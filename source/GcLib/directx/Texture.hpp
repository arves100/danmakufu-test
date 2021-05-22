#ifndef __DIRECTX_TEXTURE__
#define __DIRECTX_TEXTURE__

#include "DirectGraphics.hpp"
#include "DxConstant.hpp"

namespace directx {

class TextureData;
class Texture;
class TextureManager;
class TextureInfoPanel;

/**********************************************************
//Texture
**********************************************************/
class TextureData {
	friend Texture;
	friend TextureManager;
	friend TextureInfoPanel;

public:
	enum {
		TYPE_TEXTURE,
		TYPE_RENDER_TARGET,
	};

public:
	TextureData();
	virtual ~TextureData();
	std::wstring GetName() { return name_; }
	D3DXIMAGE_INFO GetImageInfo() { return infoImage_; }

protected:
	int type_;
	TextureManager* manager_;
	IDirect3DTexture9* pTexture_;
	D3DXIMAGE_INFO infoImage_;
	std::wstring name_;
	volatile bool bLoad_;

	IDirect3DSurface9* lpRenderSurface_; //バックバッファ実体(レンダリングターゲット用)
	IDirect3DSurface9* lpRenderZ_; //バックバッファのZバッファ実体(レンダリングターゲット用)
};

class Texture : public gstd::FileManager::LoadObject {
	friend TextureData;
	friend TextureManager;
	friend TextureInfoPanel;

public:
	Texture();
	Texture(Texture* texture);
	virtual ~Texture();
	void Release();

	std::wstring GetName();
	bool CreateFromFile(std::string path);
	bool CreateRenderTarget(std::string name);
	bool CreateFromFileInLoadThread(std::string path, bool bLoadImageInfo = false);

	void SetTexture(IDirect3DTexture9* pTexture);
	IDirect3DTexture9* GetD3DTexture();
	IDirect3DSurface9* GetD3DSurface();
	IDirect3DSurface9* GetD3DZBuffer();

	int GetWidth();
	int GetHeight();
	bool IsLoad() { return data_ != NULL && data_->bLoad_; }

protected:
	std::unique_ptr<TextureData> data_;
};

/**********************************************************
	//TextureManager
	**********************************************************/
class TextureManager : public DirectGraphicsListener, public gstd::FileManager::LoadThreadListener {
	friend Texture;
	friend TextureData;
	friend TextureInfoPanel;
	static TextureManager* thisBase_;

public:
	static const std::string TARGET_TRANSITION;

public:
	TextureManager();
	virtual ~TextureManager();
	static TextureManager* GetBase() { return thisBase_; }
	virtual bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	virtual void Clear();
	virtual void Add(std::string name, std::shared_ptr<Texture> texture); //テクスチャの参照を保持します
	virtual void Release(std::string name); //保持している参照を解放します
	virtual bool IsDataExists(std::string name);

	virtual void ReleaseDirectGraphics() { ReleaseDxResource(); }
	virtual void RestoreDirectGraphics() { RestoreDxResource(); }
	void ReleaseDxResource();
	void RestoreDxResource();

	std::shared_ptr<TextureData> GetTextureData(std::string name);
	std::shared_ptr<Texture> CreateFromFile(std::string path); //テクスチャを読み込みます。TextureDataは保持しますが、Textureは保持しません。
	std::shared_ptr<Texture> CreateRenderTarget(std::string name);
	std::shared_ptr<Texture> GetTexture(std::string name); //作成済みのテクスチャを取得します
	std::shared_ptr<Texture> CreateFromFileInLoadThread(std::wstring path, bool bLoadImageInfo = false);
	virtual void CallFromLoadThread(std::unique_ptr<gstd::FileManager::LoadThreadEvent>& event);

	void SetInfoPanel(std::shared_ptr<TextureInfoPanel> panel) { panelInfo_ = panel; }

protected:
	gstd::CriticalSection lock_;
	std::map<std::string, std::shared_ptr<Texture>> mapTexture_;
	std::map<std::string, std::shared_ptr<TextureData>> mapTextureData_;
	std::shared_ptr<TextureInfoPanel> panelInfo_;

	void _ReleaseTextureData(std::string name);
	bool _CreateFromFile(std::string path);
	bool _CreateRenderTarget(std::string name);
};

} // namespace directx

#endif
