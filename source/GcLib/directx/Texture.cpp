#include "Texture.hpp"
#include "DirectGraphics.hpp"
#include "DxUtility.hpp"

using namespace gstd;
using namespace directx;

static void bimgImageFree(void* _ptr, void* _userData)
{
	bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
	bimg::imageFree(imageContainer);
}

/**********************************************************
//Texture
**********************************************************/
Texture::Texture() = default;

Texture::Texture(Texture* texture)
{
	Lock lock(TextureManager::GetBase()->GetLock());
	data_ = texture->data_;
}

Texture::~Texture()
{
	Release();
}

void Texture::Release()
{
	if (data_ != nullptr)
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		TextureManager* manager = TextureManager::GetBase();
		if (manager != nullptr && manager->IsDataExists(data_->Name)) {
			//自身とTextureManager内の数だけになったら削除
			if (data_.use_count() == 2) {
				manager->_ReleaseTextureData(data_->Name);
			}

			data_.reset();
		}
	}
}

bool Texture::CreateFromFile(std::string path)
{
	path = StringUtility::ConvertWideToMulti(PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8)), CP_UTF8);

	Lock lock(TextureManager::GetBase()->GetLock());
	Release();
	TextureManager* manager = TextureManager::GetBase();
	if (manager->CreateDataFromFile(path, data_))
		return true;

	return false;
}

bool Texture::CreateFromFileInLoadThread(std::string path, bool bLoadImageInfo)
{
	path = StringUtility::ConvertWideToMulti(PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8)), CP_UTF8);
	
	Lock lock(TextureManager::GetBase()->GetLock());
	if (data_ != nullptr)
		Release();
	TextureManager* manager = TextureManager::GetBase();
	auto texture = manager->CreateFromFileInLoadThread(path, bLoadImageInfo);
	if (texture != nullptr) {
		data_ = texture->data_;
	}
	
	return data_ != nullptr;
}

bgfx::TextureHandle Texture::GetHandle() const
{
	bgfx::TextureHandle res = BGFX_INVALID_HANDLE;
	{
		bool bWait = true;
		const auto time = bx::getHPFrequency();
		while (bWait) {
			Lock lock(TextureManager::GetBase()->GetLock());
			bWait = !IsLoad();

			if (!bWait)
				res = data_->Handle;
			
			if (bWait && bx::getHPFrequency() - time > 10000) {
				//一定時間たってもだめだったらロック？ame; // TODO ,.,.,,.,.
				Logger::WriteTop(
					StringUtility::Format(L"テクスチャ読み込みを行えていません。ロック？ ：%S", data_->Name));
				break;
			}

			if (bWait)
				bx::sleep(1);
		}
	}
	
	return res;
}

/**********************************************************
//FrameBuffer
**********************************************************/
FrameBuffer::FrameBuffer() = default;

FrameBuffer::FrameBuffer(FrameBuffer* texture)
{
	data_ = texture->data_;
}

FrameBuffer::~FrameBuffer()
{
	Release();
}

bool FrameBuffer::Create(std::string name)
{
	TextureManager* manager = TextureManager::GetBase();
	Lock lock(manager->GetLock());
	if (data_ != nullptr)
		Release();
	return manager->CreateRenderTarget(name, data_);
}


void FrameBuffer::Release()
{
	TextureManager* manager = TextureManager::GetBase();
	if (manager != nullptr)
	{
		Lock lock(manager->GetLock());
		if (data_ != nullptr && manager->IsFrameBufferExists(data_->Name)) {
			//自身とTextureManager内の数だけになったら削除
			if (data_.use_count() == 2) {
				manager->_ReleaseFrameBufferData(data_->Name);
			}

			data_.reset();
		}
	}
}

/**********************************************************
//TextureManager
**********************************************************/
const std::string TextureManager::TARGET_TRANSITION = "__RENDERTARGET_TRANSITION__";
const std::string TextureManager::DEFAULT_FRAMEBUFFER = "__APPLICATION_DEFAULT_FRAMEBUFFER__";

TextureManager* TextureManager::thisBase_ = nullptr;

TextureManager::TextureManager() = default;

TextureManager::~TextureManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->RemoveDirectGraphicsListener(this);
	Clear();

	FileManager::GetBase()->RemoveLoadThreadListener(this);

	thisBase_ = nullptr;
}

bool TextureManager::Initialize()
{
	if (thisBase_ != nullptr)
		return false;

	thisBase_ = this;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->AddDirectGraphicsListener(this);
	FileManager::GetBase()->AddLoadThreadListener(this);

	auto tmp = std::make_shared<FrameBuffer>();

	if (!tmp->Create(TARGET_TRANSITION))
		return false;
	
	Add(TARGET_TRANSITION, tmp);

	tmp = std::make_shared<FrameBuffer>();

	if (!tmp->Create(DEFAULT_FRAMEBUFFER))
		return false;

	Add(DEFAULT_FRAMEBUFFER, tmp);
	
	return true;
}
void TextureManager::Clear()
{
	Lock lock(lock_);
	mapFrameBuffer_.clear();

	for (auto& data : mapFrameBufferData_)
	{
		_ReleaseFrameBufferData(data.first);

		if (mapFrameBufferData_.size() == 0)
			break;
	}
	
	mapFrameBufferData_.clear();
	mapTexture_.clear();

	for (auto& data : mapTextureData_)
	{
		_ReleaseTextureData(data.first);	

		if (mapTextureData_.size() == 0)
			break;
	}
	
	mapTextureData_.clear();
}

void TextureManager::_ReleaseTextureData(std::string name)
{
	Lock lock(lock_);
	if (IsDataExists(name))
	{
		auto& ptr = mapTextureData_[name];
		bgfx::destroy(ptr->Handle);
		mapTextureData_.erase(name);
		Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを解放しました[%s]", name.c_str()));
	}
}

void TextureManager::_ReleaseFrameBufferData(std::string name)
{
	Lock lock(lock_);
	if (IsFrameBufferExists(name))
	{
		auto& ptr = mapFrameBufferData_[name];
		bgfx::destroy(ptr->Handle);
		mapFrameBufferData_.erase(name);
		Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを解放しました[%s]", name.c_str()));
	}
}

void TextureManager::ReleaseDxResource()
{
	Lock lock(GetLock());
	for (auto& fb : mapFrameBufferData_)
	{
		bgfx::destroy(fb.second->Handle);
		fb.second->Handle = BGFX_INVALID_HANDLE;
	}
}

void TextureManager::RestoreDxResource()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	Lock lock(GetLock());

	for (auto& fb : mapFrameBufferData_)
	{
		/*if (graphics->GetScreenMode() == DirectGraphics::SCREENMODE_FULLSCREEN)
			fmt = graphics->GetFullScreenPresentParameter().BackBufferFormat;
		else
			fmt = graphics->GetWindowPresentParameter().BackBufferFormat;*/ // TODO

		auto data = fb.second;
		data->Handle = bgfx::createFrameBuffer(data->Width, data->Height, bgfx::TextureFormat::RGBA8);
		if (bgfx::isValid(data->Handle))
			bgfx::setName(data->Handle, data->Name.c_str());
	}
}

bool TextureManager::_CreateFromFile(std::string path)
{
	if (IsDataExists(path))
		return true;

	//まだ作成されていないなら、作成
	try {
		ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(StringUtility::ConvertMultiToWide(path, CP_UTF8));
		if (reader == NULL)
			throw std::runtime_error("ファイルが見つかりません");
		if (!reader->Open())
			throw std::runtime_error("ファイルが開けません");

		const auto size = reader->GetFileSize();
		ByteBuffer buf;
		buf.SetSize(size);
		reader->Read(buf.GetPointer(), size);
		
		bimg::ImageContainer* container = bimg::imageParse(DxAllocator::Get(), buf.GetPointer(), buf.GetSize());

		if (!container)
			return false;

		const auto mem = bgfx::makeRef(container->m_data, container->m_size, bimgImageFree, container);
		buf.Clear();

		auto data = std::make_shared<TextureData>();
		data->Width = static_cast<uint16_t>(container->m_width);
		data->Height = static_cast<uint16_t>(container->m_height);
		data->Handle = bgfx::createTexture2D(data->Width, data->Height, false, container->m_numLayers, static_cast<bgfx::TextureFormat::Enum>(container->m_format), 0, mem); // TODO: flags?
		data->Name = path;
		
		if (!bgfx::isValid(data->Handle)) {
			throw gstd::wexception(L"D3DXCreateTextureFromFileInMemoryEx失敗");
		}

		bgfx::setName(data->Handle, path.c_str());

		
		mapTextureData_[path] = data;
		
		Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを読み込みました[%s]", path.c_str()));
	} catch (gstd::wexception& e)
	{
		const std::wstring str = StringUtility::Format(L"TextureManager：テクスチャ読み込み失敗[%s]\n\t%s", path.c_str(), e.what());
		Logger::WriteTop(str);
		return false;
	}
	
	return true;
}

bool TextureManager::_CreateRenderTarget(std::string name)
{
	if (IsFrameBufferExists(name))
	{
		return true;
	}

	bool res = true;
	try
	{
		DirectGraphics* graphics = DirectGraphics::GetBase();
		const auto screenWidth = graphics->GetRenderWidth();
		const auto screenHeight = graphics->GetRenderHeight();
		uint16_t width = 2, height = 2;
		
		while (width <= screenWidth)
		{
			width *= 2;
		}
		while (height <= screenHeight)
		{
			height *= 2;
		}

		auto handle = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA8);
	
		if (!bgfx::isValid(handle))
		{
			//テクスチャを正方形にする
			if (width > height)
				height = width;
			else if (height > width)
				width = height;
			
			handle = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA8);
			if (!bgfx::isValid(handle))
				return false;
		}

		bgfx::setName(handle, name.c_str());
		
		auto ptr = std::make_shared<FrameBufferData>();
		ptr->Width = width;
		ptr->Height = height;
		ptr->Name = name;
		ptr->Handle = handle;

		mapFrameBufferData_[name] = ptr;

		Logger::WriteTop(StringUtility::Format(L"TextureManager：レンダリングターゲット作成[%s]", name.c_str()));

	}
	catch (...)
	{
		Logger::WriteTop(StringUtility::Format(L"TextureManager：レンダリングターゲット作成失敗[%s]", name.c_str()));
		res = false;
	}
	return res;
}

bool TextureManager::CreateDataFromFile(std::string path, std::shared_ptr<TextureData>& out)
{
	path = StringUtility::ConvertWideToMulti(PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8)), CP_UTF8); // TODO!

	Lock lock(lock_);
	if (mapTextureData_.find(path) != mapTextureData_.end())
	{
		out = mapTextureData_[path];
		return true;
	}

	if (_CreateFromFile(path))
	{
		out = mapTextureData_[path];
		return true;
	}

	return false;
}

bool TextureManager::CreateRenderTarget(std::string name, std::shared_ptr<FrameBufferData>& data)
{
	Lock lock(lock_);
	if (mapFrameBufferData_.find(name) != mapFrameBufferData_.end())
	{
		data = mapFrameBufferData_[name];
		return true;
	}
	
	if (_CreateRenderTarget(name))
	{
		data = mapFrameBufferData_[name];
		return true;
	}

	return false;
}

gstd::ref_count_ptr<Texture> TextureManager::CreateFromFileInLoadThread(std::string path, bool bLoadImageInfo)
{
	path = StringUtility::ConvertWideToMulti(PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8)), CP_UTF8);
	const auto wpath = StringUtility::ConvertMultiToWide(path, CP_UTF8);
	gstd::ref_count_ptr<Texture> res;
	{
		Lock lock(lock_);
		if (mapTexture_.find(path) != mapTexture_.end()) {
			res = mapTexture_[path];
		} else {
			bool bLoadTarget = true;
			res = new Texture();
			if (!IsDataExists(path)) {
				auto data = std::make_shared<TextureData>();
				mapTextureData_[path] = data;
				data->Name = path;
				data->Handle = BGFX_INVALID_HANDLE;

				//画像情報だけ事前に読み込み
				if (bLoadImageInfo) {
					try {
						ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(wpath);
						if (reader == nullptr)
							throw gstd::wexception(L"ファイルが見つかりません");
						if (!reader->Open())
							throw std::runtime_error("ファイルが開けません");

						int size = reader->GetFileSize();
						ByteBuffer buf;
						buf.SetSize(size);
						reader->Read(buf.GetPointer(), size);

						bimg::ImageContainer* container = bimg::imageParse(DxAllocator::Get(), buf.GetPointer(), buf.GetSize());

						if (!container)
							return false;

						const auto mem = bgfx::makeRef(container->m_data, container->m_size, bimgImageFree, container);
						buf.Clear();

						data->Width = static_cast<uint16_t>(container->m_width);
						data->Height = static_cast<uint16_t>(container->m_height);
						data->Handle = bgfx::createTexture2D(data->Width, data->Height, false, container->m_numLayers, static_cast<bgfx::TextureFormat::Enum>(container->m_format), 0, mem); // TODO: flags?

						if (!bgfx::isValid(data->Handle)) {
							throw gstd::wexception(L"D3DXCreateTextureFromFileInMemoryEx失敗");
						}

						bgfx::setName(data->Handle, path.c_str());

					} catch (gstd::wexception& e) {
						std::wstring str = StringUtility::Format(L"TextureManager：テクスチャ読み込み失敗[%s]\n\t%s", path.c_str(), e.what());
						Logger::WriteTop(str);
						data->Handle = BGFX_INVALID_HANDLE; //読み込み完了扱い
						bLoadTarget = false;
					}
				}
			} else
				bLoadTarget = false;

			res->data_ = mapTextureData_[path];
			if (bLoadTarget) {
				ref_count_ptr<FileManager::LoadObject> source = res;
				ref_count_ptr<FileManager::LoadThreadEvent> event = new FileManager::LoadThreadEvent(this, wpath, res);
				FileManager::GetBase()->AddLoadThreadEvent(event);
			}
		}
	}
	return res;
}

void TextureManager::CallFromLoadThread(ref_count_ptr<FileManager::LoadThreadEvent> event)
{
#if 0
	std::wstring path = event->GetPath();
	{
		Lock lock(lock_);
		auto src = event->GetSource();
		ref_count_ptr<Texture> texture = ref_count_ptr<Texture>::DownCast(src);
		if (texture == NULL)
			return;

		std::shared_ptr<TextureData> data = texture->data_;
		if (data == NULL || data->bLoad_)
			return;

		int countRef = data.GetReferenceCount();
		//自身とTextureManager内の数だけになったら読み込まない。
		if (countRef <= 2) {
			data->bLoad_ = true; //念のため読み込み完了扱い
			return;
		}

		try {
			std::shared_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
			if (reader == NULL)
				throw std::runtime_error("ファイルが見つかりません");
			if (!reader->Open())
				throw std::runtime_error("ファイルが開けません");

			int size = reader->GetFileSize();
			ByteBuffer buf;
			buf.SetSize(size);
			reader->Read(buf.GetPointer(), size);

			D3DCOLOR colorKey = D3DCOLOR_ARGB(255, 0, 0, 0);
			if (path.find(L".bmp") == std::wstring::npos) //bmpのみカラーキー適応
				colorKey = 0;

			D3DFORMAT pixelFormat = D3DFMT_A8R8G8B8;

			HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(DirectGraphics::GetBase()->GetDevice(),
				buf.GetPointer(), size,
				D3DX_DEFAULT, D3DX_DEFAULT,
				0,
				0,
				pixelFormat,
				D3DPOOL_MANAGED,
				D3DX_FILTER_BOX,
				D3DX_DEFAULT,
				colorKey,
				NULL,
				NULL,
				&data->pTexture_);
			if (FAILED(hr)) {
				throw std::runtime_error("D3DXCreateTextureFromFileInMemoryEx失敗");
			}

			D3DXGetImageInfoFromFileInMemory(buf.GetPointer(), size, &data->infoImage_);

			Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを読み込みました(LT)[%s]", path.c_str()));
		} catch (gstd::wexception& e) {
			std::wstring str = StringUtility::Format(L"TextureManager：テクスチャ読み込み失敗(LT)[%s]\n\t%s", path.c_str(), e.what());
			Logger::WriteTop(str);
		}
		data->bLoad_ = true;
	}
#endif

}

bool TextureManager::GetTextureData(std::string name, std::shared_ptr<TextureData>& out)
{
	Lock lock(lock_); // cant be const...
	if (mapTextureData_.find(name) != mapTextureData_.end())
	{
		out = mapTextureData_[name];
		return true;
	}

	return false;
}

bool TextureManager::GetFrameBuffer(std::string name, std::shared_ptr<FrameBuffer>& data)
{

	Lock lock(lock_);
	if (mapFrameBuffer_.find(name) != mapFrameBuffer_.end())
	{
		data = mapFrameBuffer_[name];
		return true;
	}

	return false;
}

void TextureManager::Add(std::string name, std::shared_ptr<FrameBuffer>& texture)
{
	Lock lock(lock_);
	if (mapFrameBuffer_.find(name) == mapFrameBuffer_.end())
	{
		mapFrameBuffer_[name] = texture;
	}
}

void TextureManager::ReleaseFrameBuffer(std::string name)
{
	Lock lock(lock_);
	mapFrameBuffer_.erase(name);
}

bool TextureManager::IsDataExists(std::string name)
{
	Lock lock(lock_);
	return mapTextureData_.find(name) != mapTextureData_.end();
}

bool TextureManager::IsFrameBufferExists(std::string name)
{
	Lock lock(lock_);
	return mapFrameBufferData_.find(name) != mapFrameBufferData_.end();
}
