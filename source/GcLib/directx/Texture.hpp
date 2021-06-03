#ifndef __DIRECTX_TEXTURE__
#define __DIRECTX_TEXTURE__

#include "DirectGraphics.hpp"
#include "DxConstant.hpp"

namespace directx
{
class Texture;
class TextureManager;

/**********************************************************
//Texture
**********************************************************/
struct TextureData
{
	uint16_t Width, Height;
	bgfx::TextureHandle Handle;
	std::string Name;
};

class Texture : public gstd::FileManager::LoadObject {
	friend TextureManager;

public:
	Texture();
	Texture(Texture* texture);
	virtual ~Texture();
	void Release();

	std::string GetName() const { return data_->Name; }
	bool CreateFromFile(std::string path);

	bool CreateFromFileInLoadThread(std::string path, bool bLoadImageInfo = false);

	bgfx::TextureHandle GetHandle() const;

	int GetWidth() const { return data_->Width; }
	int GetHeight() const { return data_->Height; }
	bool IsLoad() const { return bgfx::isValid(data_->Handle); }

protected:
	std::shared_ptr<TextureData> data_;
};

/**********************************************************
//FrameBuffer
**********************************************************/
struct FrameBufferData
{
	uint16_t Width, Height;
	bgfx::FrameBufferHandle Handle;
	std::string Name;
};

class FrameBuffer {
	friend TextureManager;

public:
	FrameBuffer();
	FrameBuffer(FrameBuffer* texture);
	virtual ~FrameBuffer();
	void Release();

	std::string GetName() const { return data_->Name; }
	bool Create(std::string path);

	bgfx::FrameBufferHandle GetHandle() const { return data_->Handle; }

	int GetWidth() const { return data_->Width; }
	int GetHeight() const { return data_->Height; }
	bool IsLoad() const { return bgfx::isValid(data_->Handle); }

protected:
	std::shared_ptr<FrameBufferData> data_;
};

/**********************************************************
	//TextureManager
	**********************************************************/
class TextureManager : public DirectGraphicsListener, public gstd::FileManager::LoadThreadListener {
	friend Texture;
	friend FrameBuffer;
	static TextureManager* thisBase_;

public:
	static const std::string TARGET_TRANSITION;
	static const std::string DEFAULT_FRAMEBUFFER;

	TextureManager();
	virtual ~TextureManager();
	static TextureManager* GetBase() { return thisBase_; }
	virtual bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	virtual void Clear();
	virtual void Add(std::string name, std::shared_ptr<FrameBuffer>& texture); //テクスチャの参照を保持します
	virtual void ReleaseFrameBuffer(std::string name); //保持している参照を解放します
	virtual bool IsDataExists(std::string name);
	virtual bool IsFrameBufferExists(std::string name);

	virtual void ReleaseDirectGraphics() { ReleaseDxResource(); }
	virtual void RestoreDirectGraphics() { RestoreDxResource(); }
	void ReleaseDxResource();
	void RestoreDxResource();

	bool GetTextureData(std::string name, std::shared_ptr<TextureData>& out);
	bool CreateDataFromFile(std::string path, std::shared_ptr<TextureData>& out);
	bool CreateRenderTarget(std::string name, std::shared_ptr<FrameBufferData>& data);
	bool GetFrameBuffer(std::string name, std::shared_ptr<FrameBuffer>& data); //作成済みのテクスチャを取得します

	gstd::ref_count_ptr<Texture> CreateFromFileInLoadThread(std::string path, bool bLoadImageInfo = false);
	virtual void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event);

protected:
	gstd::CriticalSection lock_;
	std::map<std::string, std::shared_ptr<FrameBuffer>> mapFrameBuffer_;
	std::map<std::string, std::shared_ptr<TextureData>> mapTextureData_;
	std::map<std::string, std::shared_ptr<FrameBufferData>> mapFrameBufferData_;
	std::map<std::string, gstd::ref_count_ptr<Texture>> mapTexture_;

	void _ReleaseTextureData(std::string name);
	void _ReleaseFrameBufferData(std::string name);
	bool _CreateFromFile(std::string path);
	bool _CreateRenderTarget(std::string name);
};

} // namespace directx

#endif
