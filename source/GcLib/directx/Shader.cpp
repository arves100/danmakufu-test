#include "Shader.hpp"

#include "DxUtility.hpp"
#include "HLSL.hpp"

using namespace gstd;
using namespace directx;

static void ptrFree(void*, void* _userData)
{
	free(_userData);
}

/**********************************************************
//ShaderManager
**********************************************************/
const std::string NAME_DEFAULT_SKINNED_MESH = "__NAME_DEFAULT_SKINNED_MESH__";
ShaderManager* ShaderManager::thisBase_ = nullptr;

ShaderManager::ShaderManager() = default;

ShaderManager::~ShaderManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->RemoveDirectGraphicsListener(this);

	Clear();
}

bool ShaderManager::Initialize()
{
	if (thisBase_ != nullptr)
		return false;

	thisBase_ = this;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->AddDirectGraphicsListener(this);

	/*auto shaderSkinedMesh = std::make_shared<Shader>();
	std::string sourceSkinedMesh = HLSL_DEFAULT_SKINED_MESH;
	shaderSkinedMesh->CreateFromText(sourceSkinedMesh);
	AddShader(NAME_DEFAULT_SKINNED_MESH, shaderSkinedMesh);*/

	return true;
}

void ShaderManager::Clear()
{
	Lock lock(lock_);

	for (auto& data : mapShaderData_)
	{
		_ReleaseShaderData(data.first);
	}
	
	mapShaderData_.clear();
}

void ShaderManager::_ReleaseShaderData(std::string name)
{
	Lock lock(lock_);
	if (IsDataExists(name)) {
		auto& ptr = mapShaderData_[name];
		Logger::WriteTop(StringUtility::Format(L"ShaderManager：Shaderを解放しました(Shader Released)[%s]", name.c_str()));

		if (bgfx::isValid(ptr->Program))
			bgfx::destroy(ptr->Program);
		
		if (bgfx::isValid(ptr->VertexShader))
			bgfx::destroy(ptr->VertexShader);

		if (bgfx::isValid(ptr->FragmentationShader))
			bgfx::destroy(ptr->FragmentationShader);

		mapShaderData_.erase(name);
	}
}

bool ShaderManager::_CreateFromFile(std::string name, std::string vsh, std::string fsh)
{
	lastError_ = L"";
	if (IsDataExists(name)) {
		return true;
	}

	auto data = std::make_shared<ShaderData>();

	if (!vsh.empty())
	{
		data->VertexShader = _LoadShader(vsh);
		if (!bgfx::isValid(data->VertexShader))
		{
			const std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Vertex Shader Load Failed)：\r\n%S", vsh.c_str());
			Logger::WriteTop(log);
			lastError_ = log;
			return false;
		}
		bgfx::setName(data->VertexShader, name.c_str());
	}
	
	if (!fsh.empty())
	{
		data->FragmentationShader = _LoadShader(fsh);
		if (!bgfx::isValid(data->FragmentationShader))
		{
			const std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Fragmentation Shader Load Failed)：\r\n%S", fsh.c_str());
			Logger::WriteTop(log);
			lastError_ = log;
			return false;
		}
		bgfx::setName(data->FragmentationShader, name.c_str());
	}

	data->Program = bgfx::createProgram(data->VertexShader, data->FragmentationShader, false);

	if (!bgfx::isValid(data->Program))
		return false; // invalid program??

	data->Name = name;
	
	const std::wstring log = StringUtility::Format(L"Shader読み込み(Shader Load Success)：\r\n%s", name.c_str());
	Logger::WriteTop(log);

	mapShaderData_[name] = data;
	return true;
}

bgfx::ShaderHandle ShaderManager::_LoadShader(std::string path)
{
	const auto wpath = PathProperty::GetUnique(StringUtility::ConvertMultiToWide(path, CP_UTF8));

	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(wpath);
	if (reader == nullptr || !reader->Open()) {
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s", wpath.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
		return BGFX_INVALID_HANDLE;
	}

	const auto size = reader->GetFileSize();
	ByteBuffer buf;
	buf.SetSize(size);
	reader->Read(buf.GetPointer(), size);

	const auto data = new uint8_t[size];
	memcpy(data, buf.GetPointer(), size);

	const auto ptr = bgfx::makeRef(data, size, ptrFree, data);

	return bgfx::createShader(ptr);
}

std::string ShaderManager::_GetTextSourceID(std::string& source)
{
	return StringUtility::Slice(source, 64);
}

bool ShaderManager::IsDataExists(std::string name)
{
	Lock lock(lock_);
	return mapShaderData_.find(name) != mapShaderData_.end();
}

bool ShaderManager::GetShaderData(std::string name, std::shared_ptr<ShaderData>& shader)
{
	Lock lock(lock_);
	if (mapShaderData_.find(name) != mapShaderData_.end())
	{
		shader = mapShaderData_[name];
		return true;
	}
	
	return false;
}

bool ShaderManager::CreateFromFile(std::string name, std::string vsh, std::string fsh, std::shared_ptr<ShaderData>& shader)
{
	Lock lock(lock_);
	if (_CreateFromFile(name, vsh, fsh)) {
		shader = mapShaderData_[name];
		return true;
	}

	return false;
}

std::wstring ShaderManager::GetLastError()
{
	std::wstring res;
	{
		Lock lock(lock_);
		res = lastError_;
	}
	return res;
}

/**********************************************************
//ShaderParameter
**********************************************************/
ShaderParameter::ShaderParameter() : type_(bgfx::UniformType::Count), uh_(BGFX_INVALID_HANDLE), vn_(1), data_(nullptr), dataSize_(0) {}

ShaderParameter::~ShaderParameter()
{
	Release();
}

void ShaderParameter::Release()
{
	if (bgfx::isValid(uh_))
	{
		bgfx::destroy(uh_);
		uh_ = BGFX_INVALID_HANDLE;
	}

	if (data_)
		bx::free(DxAllocator::Get(), data_);

	data_ = nullptr;
	dataSize_ = 0;
}

bool ShaderParameter::Initialize(std::string name, bgfx::UniformType::Enum type, const void* data, size_t dataSize, int16_t vn)
{
	uh_ = bgfx::createUniform(name.c_str(), type, GetValueNumber());

	if (!bgfx::isValid(uh_))
		return false;

	if (dataSize > 0)
	{
		data_ = bx::alloc(DxAllocator::Get(), dataSize); // set data later...
		bx::memCopy(data_, data, dataSize);
	}
	else
		data_ = nullptr;
	
	vn_ = vn;
	name_ = name;
	dataSize_ = dataSize;
	type_ = type;
	return true;
}

bool ShaderParameter::Set(const void* data, size_t dataSize, int16_t vn)
{
	if (vn != vn_)
	{
		bgfx::destroy(uh_);
		uh_ = bgfx::createUniform(name_.c_str(), type_, vn);
		if (!bgfx::isValid(uh_))
			return false;

		vn_ = vn;
	}
	
	if (dataSize != dataSize_)
	{
		void* ptr;
		
		if (!data_)
			ptr = bx::alloc(DxAllocator::Get(), dataSize);
		else
			ptr = bx::realloc(DxAllocator::Get(), data_, dataSize);
		
		if (!ptr)
			return false;

		dataSize_ = dataSize;
		data_ = ptr;
	}
	
	bx::memCopy(data_, data, dataSize);
	return true;
}

/**********************************************************
//TextureParameter
**********************************************************/
TextureParameter::TextureParameter() : uh_(BGFX_INVALID_HANDLE), stage_(0), tex_(BGFX_INVALID_HANDLE) {}
TextureParameter::~TextureParameter()
{
	Release();
}

void TextureParameter::Release()
{
	if (bgfx::isValid(uh_))
	{
		bgfx::destroy(uh_);
		uh_ = BGFX_INVALID_HANDLE;
	}

	tex_ = BGFX_INVALID_HANDLE;
}

bool TextureParameter::Initialize(std::string name, uint8_t stage, bgfx::TextureHandle tex)
{
	uh_ = bgfx::createUniform(name.c_str(), bgfx::UniformType::Sampler);

	if (!bgfx::isValid(uh_))
		return false;
	
	name_ = name;
	stage_ = stage;
	tex_ = tex;
	return true;
}

/**********************************************************
//Shader
**********************************************************/
Shader::Shader() = default;

Shader::Shader(Shader* shader)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	data_ = shader->data_;
}

Shader::~Shader()
{
	Release();
}

void Shader::Release()
{
	for (auto& s : mapParam_)
	{
		delete s.second;
	}

	for (auto& s : mapTex_)
	{
		delete s.second;
	}

	mapTex_.clear();
	mapParam_.clear();
	
	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr) {
		ShaderManager* manager = ShaderManager::GetBase();
		if (manager != nullptr && manager->IsDataExists(data_->Name))
		{
			auto countRef = data_.use_count();
			//自身とTextureManager内の数だけになったら削除
			if (countRef == 2)
			{
				manager->_ReleaseShaderData(data_->Name);
			}
		}
		
		data_.reset();
	}
}

void Shader::Submit(bgfx::ViewId id)
{
	for (auto& s : mapTex_)
	{
		auto& uh = s.second;
		bgfx::setTexture(uh->GetStage(), uh->GetHandle(), uh->GetTexture());
	}

	for (auto& s : mapParam_)
	{
		auto& uh = s.second;
		bgfx::setUniform(uh->GetHandle(), uh->GetValue(), uh->GetValueNumber());
	}

	
	bgfx::submit(id, data_->Program);	
}

bool Shader::CreateFromFile(std::string name, std::string vsh, std::string fsh)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr)
		Release();

	ShaderManager* manager = ShaderManager::GetBase();
	return manager->CreateFromFile(name, vsh, fsh, data_);
}

ShaderParameter* Shader::GetParameter(std::string name)
{
	const auto it = mapParam_.find(name);
	if (it == mapParam_.end())
		return nullptr;

	return mapParam_[name];
}

bool Shader::AddParameter(std::string name, bgfx::UniformType::Enum type, const void* data, size_t dataSize, int16_t vn)
{
	auto param = new ShaderParameter();

	if (!param->Initialize(name, type, data, dataSize, vn))
	{
		delete param;
		return false;
	}

	mapParam_[name] = param;
	return true;
}

bool Shader::AddTexture(std::string name, uint8_t stage, bgfx::TextureHandle handle)
{
	auto param = new TextureParameter();

	if (!param->Initialize(name, stage, handle))
	{
		delete param;
		return false;
	}

	mapTex_[stage] = param;
	return true;
}
