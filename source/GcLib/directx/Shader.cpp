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
const std::string NAME_DEFAULT_SKINNED_MESH = "internal/default_skin";
const std::string DEFAULT_SUBMIT_SHADER = "internal/view1";

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

#if 0 // TODO
	if (!_CreateFromFile(DEFAULT_SUBMIT_SHADER, false))
		return false;

	if (!_CreateFromFile(NAME_DEFAULT_SKINNED_MESH, false))
		return false;
#endif

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
		
		if (bgfx::isValid(ptr->Shader1))
			bgfx::destroy(ptr->Shader1);

		if (bgfx::isValid(ptr->Shader2))
			bgfx::destroy(ptr->Shader2);

		mapShaderData_.erase(name);
	}
}

bool ShaderManager::_CreateFromFile(std::string name, bool isComputeShader)
{
	lastError_ = L"";
	if (IsDataExists(name)) {
		return true;
	}

	auto data = std::make_shared<ShaderData>();

	if (isComputeShader)
	{
		data->Shader1 = _LoadShader(name, 2);
		if (!bgfx::isValid(data->Shader1))
		{
			const std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Compute Shader Load Failed)：\r\n%S", name.c_str());
			Logger::WriteTop(log);
			lastError_ = log;
			return false;
		}
		bgfx::setName(data->Shader1, (name + "_CS").c_str());

		data->Program = bgfx::createProgram(data->Shader1, false);
	}
	else
	{
		data->Shader1 = _LoadShader(name, 0);
		if (!bgfx::isValid(data->Shader1))
		{
			const std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Vertex Shader Load Failed)：\r\n%S", name.c_str());
			Logger::WriteTop(log);
			lastError_ = log;
			return false;
		}
		bgfx::setName(data->Shader1, (name + "_VS").c_str());

		data->Shader2 = _LoadShader(name, 1);
		if (!bgfx::isValid(data->Shader2))
		{
			const std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Fragmentation Shader Load Failed)：\r\n%S", name.c_str());
			Logger::WriteTop(log);
			lastError_ = log;
			return false;
		}
		bgfx::setName(data->Shader2, (name + "_FS").c_str());

		data->Program = bgfx::createProgram(data->Shader1, data->Shader2, false);
	}

	if (!bgfx::isValid(data->Program))
		return false; // invalid program??

	data->Name = name;
	
	const std::wstring log = StringUtility::Format(L"Shader読み込み(Shader Load Success)：\r\n%s", name.c_str());
	Logger::WriteTop(log);

	mapShaderData_[name] = data;
	return true;
}

bgfx::ShaderHandle ShaderManager::_LoadShader(std::string path, uint8_t type) /* internal function */
{
	std::wstring sh_type; /* TODO: when things get replaced, replace wstring to string */
	switch (type)
	{
	case 0: /* vertex shader */
		sh_type = L"vs";
		break;
	case 1: /* fragmentation shader */
		sh_type = L"fs";
		break;
	case 2: /* compute shader */
		sh_type = L"cs";
		break;
	default:
		return BGFX_INVALID_HANDLE;
	}

	std::wstring render_name;
	switch (bgfx::getRendererType())
	{
	case bgfx::RendererType::Direct3D9:
		render_name = L"hlsl3";
		break;
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12:
		render_name = L"hlsl5";
		break;
	case bgfx::RendererType::Gnm:
		render_name = L"pssl";
		break;
	case bgfx::RendererType::Vulkan:
		render_name = L"spirv";
		break;
	case bgfx::RendererType::Metal:
		render_name = L"metal";
		break;
	case bgfx::RendererType::OpenGL:
		render_name = L"glsl";
		break;
	case bgfx::RendererType::OpenGLES:
		render_name = L"essl";
		break;
	default:
		return BGFX_INVALID_HANDLE;
	}

	// TODO: make a file format for compressing all this shaders

	const auto wpath = PathProperty::GetUnique(StringUtility::Format(L"shaders/%s/%s_%s.bin", render_name.c_str(), StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str(), sh_type.c_str()));

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

bool ShaderManager::CreateFromFile(std::string name, bool isComputeShader, std::shared_ptr<ShaderData>& shader)
{
	Lock lock(lock_);
	if (_CreateFromFile(name, isComputeShader)) {
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

		if (mapParam_.size() == 0)
			break;
	}

	for (auto& s : mapTex_)
	{
		delete s.second;

		if (mapTex_.size() == 0)
			break;
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

	DirectGraphics::GetBase()->Submit(id, data_->Program);
}

bool Shader::CreateFromFile(std::string name, bool isComputeShader)
{
	Lock lock(ShaderManager::GetBase()->GetLock());
	if (data_ != nullptr)
		Release();

	ShaderManager* manager = ShaderManager::GetBase();
	return manager->CreateFromFile(name, isComputeShader, data_);
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
	const auto& p = mapParam_.find(name);

	if (p == mapParam_.end())
	{
		auto param = new ShaderParameter();

		if (!param->Initialize(name, type, data, dataSize, vn))
		{
			delete param;
			return false;
		}

		mapParam_[name] = param;
	}
	else
	{
		p->second->Set(data, dataSize, vn);
	}
	return true;
}

bool Shader::AddTexture(uint8_t stage, bgfx::TextureHandle handle)
{
	const auto& p = mapTex_.find(stage);

	if (p == mapTex_.end())
	{
		auto param = new TextureParameter();

		if (!param->Initialize(_GetTextureNameFromStage(stage), stage, handle))
		{
			delete param;
			return false;
		}

		mapTex_[stage] = param;
	}
	else
		p->second->tex_ = handle;

	return true;
}

std::string Shader::_GetTextureNameFromStage(uint8_t stage)
{
	switch (stage)
	{
	case 0:
		return "s_texDiffuse";
	case 1:
		return "s_texNormal";
	default:
		break;
	}

	return "s_texUnk_" + std::to_string(stage);
}
