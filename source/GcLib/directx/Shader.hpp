#ifndef __DIRECTX_SHADER__
#define __DIRECTX_SHADER__

#include "DirectGraphics.hpp"
#include "DxConstant.hpp"
#include "Texture.hpp"

namespace directx {

// http://msdn.microsoft.com/ja-jp/library/bb944006(v=vs.85).aspx
// http://msdn.microsoft.com/ja-jp/library/bb509647(v=vs.85).aspx

class ShaderManager;
class Shader;
class ShaderData;
/**********************************************************
//ShaderData
**********************************************************/
struct ShaderData
{
	ShaderData() : VertexShader(BGFX_INVALID_HANDLE), FragmentationShader(BGFX_INVALID_HANDLE), Program(BGFX_INVALID_HANDLE) {}
	
	virtual ~ShaderData()
	{
		if (bgfx::isValid(Program))
			bgfx::destroy(Program);
		
		if (bgfx::isValid(VertexShader))
			bgfx::destroy(VertexShader);

		if (bgfx::isValid(FragmentationShader))
			bgfx::destroy(FragmentationShader);
	}

	bgfx::ShaderHandle VertexShader;
	bgfx::ShaderHandle FragmentationShader;
	bgfx::ProgramHandle Program;
	std::string Name;
};

/**********************************************************
//ShaderManager
**********************************************************/
class ShaderManager : public DirectGraphicsListener {
	friend Shader;

public:
	ShaderManager();
	virtual ~ShaderManager();
	static ShaderManager* GetBase() { return thisBase_; }
	virtual bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }
	void Clear();

#if 0
	virtual void ReleaseDirectGraphics() { ReleaseDxResource(); }
	virtual void RestoreDirectGraphics() { RestoreDxResource(); }
	void ReleaseDxResource();
	void RestoreDxResource();
#endif
	
	virtual bool IsDataExists(std::string name);
	bool GetShaderData(std::string name, std::shared_ptr<ShaderData>& shader);
	bool CreateFromFile(std::string path, std::shared_ptr<Shader>& shader); //読み込みます。ShaderDataは保持しますが、Shaderは保持しません。
	bool CreateFromText(std::string source, std::shared_ptr<Shader>& shader); //読み込みます。ShaderDataは保持しますが、Shaderは保持しません。

	void AddShader(std::string name, std::shared_ptr<Shader>& shader);
	void DeleteShader(std::string name);
	bool GetShader(std::string name, std::shared_ptr<Shader>& shader);
	std::shared_ptr<Shader>& GetDefaultSkinnedMeshShader();

	std::wstring GetLastError();

protected:
	gstd::CriticalSection lock_;
	std::map<std::string, std::shared_ptr<Shader>> mapShader_;
	std::map<std::string, std::shared_ptr<ShaderData>> mapShaderData_;

	std::wstring lastError_;

	void _ReleaseShaderData(std::string name);
	bool _CreateFromFile(std::string path);
	bool _CreateFromText(std::string& source);
	void _BeginShader(Shader* shader, int pass);
	void _EndShader(Shader* shader);
	static std::string _GetTextSourceID(std::string& source);

private:
	static ShaderManager* thisBase_;
};

// TODO wdym ?
#if 0
/**********************************************************
//ShaderParameter
**********************************************************/
class ShaderParameter {
public:
	enum {
		TYPE_UNKNOWN,
		TYPE_MATRIX,
		TYPE_MATRIX_ARRAY,
		TYPE_VECTOR,
		TYPE_FLOAT,
		TYPE_FLOAT_ARRAY,
		TYPE_TEXTURE,
	};

public:
	ShaderParameter();
	virtual ~ShaderParameter();

	int GetType() { return type_; }
	void SetMatrix(D3DXMATRIX matrix);
	D3DXMATRIX GetMatrix();
	void SetMatrixArray(std::vector<D3DXMATRIX>& matrix);
	std::vector<D3DXMATRIX> GetMatrixArray();
	void SetVector(D3DXVECTOR4 vector);
	D3DXVECTOR4 GetVector();
	void SetFloat(float value);
	float GetFloat();
	void SetFloatArray(std::vector<float>& values);
	std::vector<float> GetFloatArray();
	void SetTexture(gstd::ref_count_ptr<Texture> texture);
	gstd::ref_count_ptr<Texture> GetTexture();

private:
	int type_;
	gstd::ref_count_ptr<gstd::ByteBuffer> value_;
	gstd::ref_count_ptr<Texture> texture_;
};
#endif

/**********************************************************
//Shader
**********************************************************/
class Shader {
	friend ShaderManager;

public:
	Shader();
	Shader(Shader* shader);
	virtual ~Shader();
	void Release();

	void Submit();

	bool CreateFromFile(std::string path);
	bool CreateFromText(std::string& source);
	bool IsLoad() const { return data_ != nullptr && bgfx::isValid(data_->Program); }

#if 0
	bool SetMatrix(std::string name, glm::mat4 matrix);
	bool SetMatrixArray(std::string name, std::vector<glm::mat4>& matrix);
	bool SetVector(std::string name, glm::vec4 vector);
	bool SetFloat(std::string name, float value);
	bool SetFloatArray(std::string name, std::vector<float>& values);
	bool SetTexture(std::string name, gstd::ref_count_ptr<Texture> texture);
#endif

protected:
	std::shared_ptr<ShaderData> data_;
	std::map<std::string, bgfx::UniformHandle> params_;

	ShaderData& _GetShaderData() const { return *data_; }
};

} // namespace directx

#endif
