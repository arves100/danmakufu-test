#ifndef __DIRECTX_SHADER__
#define __DIRECTX_SHADER__

#include <unordered_map>

#include "DirectGraphics.hpp"
#include "DxConstant.hpp"
#include "Texture.hpp"

namespace directx {

class ShaderManager;
class Shader;
class IRenderObject;
struct ShaderData;
	
/**********************************************************
//ShaderData
**********************************************************/
struct ShaderData
{
	ShaderData() : Shader1(BGFX_INVALID_HANDLE), Shader2(BGFX_INVALID_HANDLE), Program(BGFX_INVALID_HANDLE) {}

	bgfx::ShaderHandle Shader1; // vs o cs
	bgfx::ShaderHandle Shader2; // fs
	bgfx::ProgramHandle Program;
	std::string Name;
};

/**********************************************************
//ShaderManager
**********************************************************/
class ShaderManager : public DirectGraphicsListener {
	friend Shader;

public:
	const std::string DEFAULT_SUBMIT_SHADER;
	
	ShaderManager();
	virtual ~ShaderManager();
	static ShaderManager* GetBase() { return thisBase_; }
	virtual bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }
	void Clear();

	bool IsDataExists(std::string name);
	bool GetShaderData(std::string name, std::shared_ptr<ShaderData>& shader);
	bool CreateFromFile(std::string name, bool isComputeShader, std::shared_ptr<ShaderData>& shader); //読み込みます。ShaderDataは保持しますが、Shaderは保持しません。

	std::wstring GetLastError();

protected:
	gstd::CriticalSection lock_;
	std::map<std::string, std::shared_ptr<ShaderData>> mapShaderData_;

	std::wstring lastError_;

	void _ReleaseShaderData(std::string name);
	bool _CreateFromFile(std::string name, bool isComputeShader);
	static std::string _GetTextSourceID(std::string& source);
	bgfx::ShaderHandle _LoadShader(std::string path, uint8_t type);

private:
	static ShaderManager* thisBase_;
};


/**********************************************************
//ShaderParameter
**********************************************************/
class ShaderParameter
{
public:
	friend Shader;

	ShaderParameter();
	virtual ~ShaderParameter();
	
	bool Initialize(std::string name, bgfx::UniformType::Enum type, const void* data, size_t dataSize, int16_t vn = 1);
	bool Set(const void* data, size_t dataSize, int16_t vn = 1);
	void Release();
	
	bgfx::UniformType::Enum GetType() const { return type_; }
	const void* GetValue() const { return data_; }
	int16_t GetValueNumber() const { return vn_; }
	
	bgfx::UniformHandle GetHandle() const { return uh_; }
	std::string GetName() const { return name_; }

private:
	bgfx::UniformType::Enum type_;
	bgfx::UniformHandle uh_;
	std::string name_;
	uint16_t vn_;
	void* data_;
	size_t dataSize_;
};

class TextureParameter
{
public:
	friend Shader;

	TextureParameter();
	virtual ~TextureParameter();
	
	bgfx::UniformHandle GetHandle() const { return uh_; }
	std::string GetName() const { return name_; }
	
	bool Initialize(std::string name, uint8_t stage, bgfx::TextureHandle tex);
	void Release();

	uint8_t GetStage() const { return stage_; }
	bgfx::TextureHandle GetTexture() const { return tex_; }

private:
	bgfx::UniformHandle uh_;
	std::string name_;
	uint8_t stage_;
	bgfx::TextureHandle tex_;
};

/**********************************************************
//Shader
**********************************************************/
class Shader
{
	friend ShaderManager;
	friend DirectGraphics;

public:
	Shader();
	Shader(Shader* shader);
	virtual ~Shader();
	void Release();

	void Submit(bgfx::ViewId id = 0);

	bool CreateFromFile(std::string name, bool isComputeShader);
	bool IsLoad() const { return data_ != nullptr && bgfx::isValid(data_->Program); }

	bool AddParameter(std::string name, bgfx::UniformType::Enum type, const void* data, size_t dataSize, int16_t vn = 1);
	ShaderParameter* GetParameter(std::string name);

	bool AddTexture(uint8_t stage, bgfx::TextureHandle handle);

protected:
	std::shared_ptr<ShaderData> data_;
	std::unordered_map<std::string, ShaderParameter*> mapParam_;
	std::unordered_map<uint16_t, TextureParameter*> mapTex_;

	std::string _GetTextureNameFromStage(uint8_t stage);
};

} // namespace directx

#endif
