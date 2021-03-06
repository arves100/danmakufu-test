#ifndef __DIRECTX_RENDEROBJECT__
#define __DIRECTX_RENDEROBJECT__

#include "DxConstant.hpp"
#include "DxUtility.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexDecl.hpp"

namespace directx {

class RenderObjectBase;
class RenderManager;

class RenderStateFunction;
class RenderBlock;

/**********************************************************
//RenderBlock
**********************************************************/
#if 0
class RenderBlock {
public:
	RenderBlock();
	virtual ~RenderBlock();
	void SetRenderFunction(std::shared_ptr<RenderStateFunction> func) { func_ = func; }
	virtual void Render();

	virtual void CalculateZValue() = 0;
	float GetZValue() { return posSortKey_; }
	void SetZValue(float pos) { posSortKey_ = pos; }
	virtual bool IsTranslucent() = 0; //Zソート対象に使用

	void SetRenderObject(std::shared_ptr<RenderObject> obj) { obj_ = obj; }
	std::shared_ptr<RenderObject> GetRenderObject() { return obj_; }
	void SetPosition(D3DXVECTOR3& pos) { position_ = pos; }
	void SetAngle(D3DXVECTOR3& angle) { angle_ = angle; }
	void SetScale(D3DXVECTOR3& scale) { scale_ = scale; }

protected:
	float posSortKey_;
	std::shared_ptr<RenderStateFunction> func_;
	std::shared_ptr<RenderObject> obj_;

	D3DXVECTOR3 position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
};

class RenderBlocks {
public:
	RenderBlocks(){};
	virtual ~RenderBlocks(){};
	void Add(std::shared_ptr<RenderBlock> block) { listBlock_.push_back(block); }
	std::list<std::shared_ptr<RenderBlock>>& GetList() { return listBlock_; }

protected:
	std::list<std::shared_ptr<RenderBlock>> listBlock_;
};

/**********************************************************
//RenderManager
//レンダリング管理
//3D不透明オブジェクト
//3D半透明オブジェクトZソート順
//2Dオブジェクト
//順に描画する
**********************************************************/
class RenderManager {
	class ComparatorRenderBlockTranslucent;

public:
	RenderManager();
	virtual ~RenderManager();
	virtual void Render();
	void AddBlock(std::shared_ptr<RenderBlock> block);
	void AddBlock(std::shared_ptr<RenderBlocks> blocks);

protected:
	std::list<std::shared_ptr<RenderBlock>> listBlockOpaque_;
	std::list<std::shared_ptr<RenderBlock>> listBlockTranslucent_;
};

class RenderManager::ComparatorRenderBlockTranslucent {
public:
	bool operator()(std::shared_ptr<RenderBlock> l, std::shared_ptr<RenderBlock> r)
	{
		return l->GetZValue() > r->GetZValue();
	}
};

/**********************************************************
//RenderStateFunction
**********************************************************/
class RenderStateFunction {
	friend RenderObjectBase;

public:
	RenderStateFunction();
	virtual ~RenderStateFunction();
	void CallRenderStateFunction();

	//レンダリングステート設定(RenderManager用)
	void SetLightingEnable(bool bEnable); //ライティング
	void SetCullingMode(DWORD mode); //カリング
	void SetZBufferEnable(bool bEnable); //Zバッファ参照
	void SetZWriteEnalbe(bool bEnable); //Zバッファ書き込み
	void SetBlendMode(DWORD mode, int stage = 0);
	void SetTextureFilter(DWORD mode, int stage = 0);

private:
	enum FUNC_TYPE {
		FUNC_LIGHTING,
		FUNC_CULLING,
		FUNC_ZBUFFER_ENABLE,
		FUNC_ZBUFFER_WRITE_ENABLE,
		FUNC_BLEND,
		FUNC_TEXTURE_FILTER,
	};

	std::map<FUNC_TYPE, std::shared_ptr<gstd::ByteBuffer>> mapFuncRenderState_;
};

class Matrices {
public:
	Matrices(){};
	virtual ~Matrices(){};
	void SetSize(int size)
	{
		matrix_.resize(size);
		for (int iMat = 0; iMat < size; iMat++) {
			D3DXMatrixIdentity(&matrix_[iMat]);
		}
	}
	int GetSize() { return matrix_.size(); }
	void SetMatrix(int index, D3DXMATRIX mat) { matrix_[index] = mat; }
	D3DXMATRIX& GetMatrix(int index) { return matrix_[index]; }

private:
	std::vector<D3DXMATRIX> matrix_;
};
#endif

/**********************************************************
//RenderObject
//レンダリングオブジェクト
//描画の最小単位
//RenderManagerに登録して描画してもらう
//(直接描画も可能)
**********************************************************/
template <typename T>
class RenderObject
{
public:
	RenderObject() : posWeightCenter_(glm::vec3(0.0f, 0.0f, 0.0f)),
		pVertexBuffer_(BGFX_INVALID_HANDLE), pIndexBuffer_(BGFX_INVALID_HANDLE),
		position_(glm::vec3(0.0f, 0.0f, 0.0f)), angle_(glm::vec3(0.0f, 0.0f, 0.0f)), scale_(glm::vec3(1.0f, 1.0f, 1.0f)),
		matRelative_(glm::mat4()), bCoordinate2D_(false), typePrimitive_(bgfx::Topology::TriList), bRecreate_(true), posBias_(-0.5f), pConvertedIndices_(nullptr)
	{
		matRelative_ = glm::identity<glm::mat4>();
		T::get(pVertexDecl_);

		shader_ = std::make_unique<Shader>();

		// 2 texture stages
		texture_.reserve(2);
		texture_.resize(2);
	}
	
	virtual ~RenderObject()
	{
		_ReleaseBuffers();
	}

	virtual void CalculateWeightCenter() {}
	glm::vec3 GetWeightCenter() const { return posWeightCenter_; }

	void SetRelativeMatrix(const glm::mat4 mat) { matRelative_ = mat; }

	void SetPrimitiveType(const bgfx::Topology::Enum type) { typePrimitive_ = type; }

	//頂点設定

	virtual void SetVertexCount(const size_t count)
	{
		vertex_.reserve(count);
		vertex_.resize(count);
	}
	size_t GetVertexCount() const { return vertex_.size(); }

	void SetVertices(const T* vertices, size_t size)
	{
		SetVertexCount(size);
		bx::memCopy(&vertex_[0], vertices, size * pVertexDecl_.getStride());
	}

	bgfx::VertexLayout GetVertexDecl() const { return pVertexDecl_; }

	glm::vec3 GetScale() const { return scale_; }
	glm::vec3 GetAngle() const { return angle_; }
	
	size_t GetIndexCount() const { return vertexIndices_.size(); }

	void SetIndices(std::vector<uint16_t>& indecies) { vertexIndices_ = indecies; }
	void SetIndices(const uint16_t* indices, size_t size)
	{
		SetIndicesCount(size);
		bx::memCopy(&vertexIndices_[0], indices, size * sizeof(uint16_t));
	}
	
	void SetIndex(size_t index, uint16_t iv)
	{
		vertexIndices_[index] = iv;
	}

	void SetIndicesCount(const size_t count)
	{
		vertexIndices_.reserve(count);
		vertexIndices_.resize(count);
	}

	Texture* GetTexture(size_t i) const
	{
		if (texture_.size() < (i + 1))
			return nullptr;

		return texture_[i].get();
	}

	glm::vec3 GetPosition() const { return position_; }
	
	//描画用設定
	void SetPosition(const glm::vec3 pos) { position_ = pos; }
	void SetPosition(const float x, const float y, const float z)
	{
		position_.x = x;
		position_.y = y;
		position_.z = z;
	}
	void SetX(const float x) { position_.x = x; }
	void SetY(const float y) { position_.y = y; }
	void SetZ(const float z) { position_.z = z; }
	void SetAngle(const glm::vec3 angle) { angle_ = angle; }
	void SetAngleXYZ(const float angx = 0.0f, const float angy = 0.0f, const float angz = 0.0f)
	{
		angle_.x = angx;
		angle_.y = angy;
		angle_.z = angz;
	}
	void SetScale(const glm::vec3 scale) { scale_ = scale; }
	void SetScaleXYZ(const float sx = 1.0f, const float sy = 1.0f, const float sz = 1.0f)
	{
		scale_.x = sx;
		scale_.y = sy;
		scale_.z = sz;
	}
	
	bool SetTexture(std::unique_ptr<Texture>& texture, uint8_t stage = 0) //テクスチャ設定
	{
		if ((stage + 1) >= texture_.size())
		{
			texture_.resize(stage + 1);
			texture_.reserve(stage + 1);
		}

		if (!shader_->AddTexture(stage, texture->GetHandle()))
			return false;
		
		texture_[stage] = std::move(texture);
		return true;
	}

	bool IsCoordinate2D() const { return bCoordinate2D_; }
	void SetCoordinate2D(const bool b) { bCoordinate2D_ = b; }

	Shader* GetShader() const { return shader_.get(); }

	virtual void Submit(bgfx::ViewId id = 0)
	{
		const auto mtx = _CreateWorldTransformMatrix();
		bgfx::setTransform(&mtx[0]);

		for (auto i = 0; i < texture_.size(); i++)
			shader_->AddTexture(static_cast<uint8_t>(i), texture_[i]->GetHandle());
		
		bgfx::setVertexBuffer(0, pVertexBuffer_);

		//if (bgfx::isValid(pIndexBuffer_))
		bgfx::setIndexBuffer(pIndexBuffer_);

		shader_->Submit(id);
	}

	T* GetVertex(const size_t index) const
	{
		if (index >= vertex_.size())
			return nullptr;
		return const_cast<T*>(&vertex_[index]);
	}
	
	void SetVertex(const size_t index, const T vertex)
	{
		if (index >= vertex_.size())
			return;
		vertex_[index] = vertex;
	}

	bool GetTexture(uint8_t pos, std::shared_ptr<Texture>& ptr)
	{
		if (texture_.size() < pos)
			return false;

		ptr = texture_[pos];
		return true;
	}

	void SetTexture(Texture* texture, uint8_t stage)
	{
		if (texture == nullptr)
			texture_[stage] = nullptr;
		else
		{
			if (stage >= texture_.size())
				return;
			texture_[stage] = new Texture(texture);
		}
	}

	void SetTexture(std::shared_ptr<Texture>& texture, uint8_t stage)
	{
		if (texture == nullptr)
			texture_[stage] = nullptr;
		else
		{
			if (stage >= texture_.size())
				return;

			texture_[stage] = texture;
		}
	}

	void SetName(std::string n)
	{
		name_ = n;
	}

	bool Finalize()
	{
		_ReleaseBuffers();

		if (!shader_->IsLoad())
		{
			if (!shader_->CreateFromFile(_GetDefaultShaderName(), false))
				return false;
		}

		const bgfx::Memory* ib_mem;

		if (typePrimitive_ != bgfx::Topology::TriList)
		{
			auto conv = GetConversionPrimitive();
			const auto size = bgfx::topologyConvert(conv, nullptr, 0, vertexIndices_.data(), vertexIndices_.size(), false) + vertexIndices_.size();
			pConvertedIndices_ = new uint16_t[size];
			bx::memCopy(pConvertedIndices_, vertexIndices_.data(), vertexIndices_.size());
			bgfx::topologyConvert(conv, pConvertedIndices_, size, vertexIndices_.data(), vertexIndices_.size(), false);
			ib_mem = bgfx::makeRef(pConvertedIndices_, size * sizeof(uint16_t));
		}
		else
			ib_mem = bgfx::makeRef(vertexIndices_.data(), vertexIndices_.size() * sizeof(uint16_t));

		const auto vb_mem = bgfx::makeRef(vertex_.data(), pVertexDecl_.getSize(vertex_.size()));
		pVertexBuffer_ = bgfx::createVertexBuffer(vb_mem, pVertexDecl_, _AllowVBResize() ? BGFX_BUFFER_ALLOW_RESIZE : 0);
		bgfx::setName(pVertexBuffer_, name_.c_str());

		pIndexBuffer_ = bgfx::createIndexBuffer(ib_mem, _AllowIBResize() ? BGFX_BUFFER_ALLOW_RESIZE : 0);
		bgfx::setName(pIndexBuffer_, name_.c_str());

		bRecreate_ = false;
		return true;
	}

protected:
	std::vector<T> vertex_; //頂点
	std::vector<uint16_t> vertexIndices_;
	glm::vec3 posWeightCenter_; //重心

	//シェーダ用
	bgfx::VertexBufferHandle pVertexBuffer_;
	bgfx::IndexBufferHandle pIndexBuffer_;
	bgfx::VertexLayout pVertexDecl_;

	glm::vec3 position_; //移動先座標
	glm::vec3 angle_; //回転角度
	glm::vec3 scale_; //拡大率
	glm::mat4 matRelative_; //関係行列
	bool bCoordinate2D_; //2D座標指定
	bgfx::Topology::Enum typePrimitive_;

	std::vector<std::shared_ptr<Texture>> texture_;

	std::unique_ptr<Shader> shader_;
	bool bRecreate_;
	float posBias_;
	std::string name_;
	uint16_t* pConvertedIndices_;

	bool _AllowIBResize() const { return false; }
	bool _AllowVBResize() const { return false; }

	bgfx::TopologyConvert::Enum GetConversionPrimitive() const
	{
		switch (typePrimitive_)
		{
		case bgfx::Topology::TriStrip:
			return bgfx::TopologyConvert::TriStripToTriList;
		default:
			break;
		}

		return bgfx::TopologyConvert::Count;
	}

	void _ReleaseBuffers()
	{
		if (pConvertedIndices_)
			delete[] pConvertedIndices_;

		if (bgfx::isValid(pVertexBuffer_))
			bgfx::destroy(pVertexBuffer_);
		if (bgfx::isValid(pIndexBuffer_))
			bgfx::destroy(pIndexBuffer_);

		pVertexBuffer_ = BGFX_INVALID_HANDLE;
		pIndexBuffer_ = BGFX_INVALID_HANDLE;
		pConvertedIndices_ = nullptr;
		bRecreate_ = true;
	}

	void _SetTextureStageCount(const size_t count)
	{
		texture_.resize(count);
		for (int i = 0; i < count; i++)
			texture_[i] = nullptr;
	}
	
	virtual glm::mat4 _CreateWorldTransformMatrix() //position_,angle_,scale_から作成
	{
		glm::mat4 mat = glm::identity<glm::mat4>();

		const bool bPos = position_.x != 0.0f || position_.y != 0.0f || position_.z != 0.0f;
		const bool bAngle = angle_.x != 0.0f || angle_.y != 0.0f || angle_.z != 0.0f;
		const bool bScale = scale_.x != 1.0f || scale_.y != 1.0f || scale_.z != 1.0f;

		if (bPos || bAngle || bScale) {
			if (bScale) {
				glm::mat4 matScale(1.0f);
				matScale = glm::scale(matScale, scale_);
				mat = mat * matScale;
			}
			if (bAngle) {
				const glm::mat4 matRot = glm::yawPitchRoll(glm::radians(angle_.y), glm::radians(angle_.x), glm::radians(angle_.z));
				mat = mat * matRot;
			}

			if (bPos) {
				glm::mat4 matTrans(1.0f);
				matTrans = glm::translate(matTrans, position_);
				mat = mat * matTrans;
			}
		}

		mat = mat * matRelative_;

		if (bCoordinate2D_)
		{
			DirectGraphics* graphics = DirectGraphics::GetBase();

			const auto width = static_cast<float>(graphics->GetRenderWidth());
			const auto height = static_cast<float>(graphics->GetRenderHeight());

			const auto camera2D = graphics->GetCamera2D();
			if (camera2D->IsEnable()) {
				mat = mat * camera2D->GetMatrix();
			}

			auto matViewPort = glm::identity<glm::mat4>();
			matViewPort[0][0] = width / 2.0f;
			matViewPort[3][0] = width / 2.0f;
			matViewPort[1][1] = -height / 2.0f;
			matViewPort[3][1] = height / 2.0f;

			auto matTrans = glm::identity<glm::mat4>();
			matTrans = glm::translate(matTrans, glm::vec3(-width / 2.0f, -height / 2.0f, 0.0f));

			auto matScale = glm::identity<glm::mat4>();
			matScale = glm::scale(matScale, glm::vec3(200.0f, 200.0f, -0.002f));

			const auto matInvView = glm::inverse(graphics->GetViewMatrix());
			const auto matInvProj = glm::inverse(graphics->GetProjMatrix());
			const auto matInvViewPort = glm::inverse(matViewPort);

			mat = mat * matTrans * matScale * matInvViewPort * matInvProj * matInvView;
			mat[3][2] *= -1;
		}

		return mat;
	}
	
	void _SetCoordinate2dDeviceMatrix() const
	{
		if (!bCoordinate2D_)
			return;

		DirectGraphics* graphics = DirectGraphics::GetBase();

		const auto viewFrom = glm::vec3(0.0f, 0.0f, -100.0f);
		const auto viewMat = glm::lookAtLH(viewFrom, glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f));
		const auto persMat = glm::perspectiveFovLH(glm::radians(180.0f), static_cast<float>(graphics->GetRenderWidth()), static_cast<float>(graphics->GetRenderHeight()), 1.0f, 2000.0f);

		graphics->SetViewAndProjMatrix(viewMat, persMat);
	}

	virtual const char* _GetDefaultShaderName() const { return nullptr; }
};

class RenderObjectLTA : public RenderObject<VERTEX_LTA> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_UV
	RO_IMPL_TANGENT
	RO_IMPL_NORMAL4

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/lta"; }
};

class RenderObjectTL : public RenderObject<VERTEX_TL> {
public:
	RO_IMPL_VERTEX4
	RO_IMPL_RGBA

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/tl"; }
};

class RenderObjectL : public RenderObject<VERTEX_L> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_RGBA

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/l"; }
};

class RenderObjectN : public RenderObject<VERTEX_N> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_NORMAL3

protected:
	const char* _GetDefaultShaderName() const override;
};

class RenderObjectNXG : public RenderObject<VERTEX_NXG> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_WEIGHT3
	RO_IMPL_NORMAL3
	RO_IMPL_UV

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/nxg"; }
};

/**********************************************************
//RenderObjectTLX
//座標3D変換済み、ライティング済み、テクスチャ有り
//2D自由変形スプライト用
**********************************************************/
class RenderObjectTLX : public RenderObject<VERTEX_TLX> {
public:
	RenderObjectTLX();

	RO_IMPL_VERTEX4
	RO_IMPL_RGBA
	RO_IMPL_UV

	void SetVertexCount(const size_t count) override;

	bool IsPermitCamera() { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }

	void Submit(bgfx::ViewId id = 0) override;

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/tlx"; }
	bool bPermitCamera_;
};

/**********************************************************
//RenderObjectLX
//ライティング済み、テクスチャ有り
//3Dエフェクト用
**********************************************************/
class RenderObjectLX : public RenderObject<VERTEX_LX> {
public:
	RenderObjectLX();

	RO_IMPL_VERTEX3
	RO_IMPL_RGBA
	RO_IMPL_UV

	void SetVertexCount(const size_t count) override;

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/lx"; }
};

/**********************************************************
//RenderObjectNX
//法線有り、テクスチャ有り
**********************************************************/
class RenderObjectNX : public RenderObject<VERTEX_NX> {
public:
	RenderObjectNX();

	RO_IMPL_VERTEX3
	RO_IMPL_NORMAL3
	RO_IMPL_UV

	void Submit(bgfx::ViewId id = 0) override;
	void SetColor(const uint32_t color) { color_ = color; }

protected:
	uint32_t color_;
	const char* _GetDefaultShaderName() const override { return "renderobject/nx"; }
};

class RenderObjectB1NX : public RenderObject<VERTEX_B1NX> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_INDICES
	RO_IMPL_NORMAL3
	RO_IMPL_UV

protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/b1nx"; }
};

/**********************************************************
//RenderObjectB2NX
//頂点ブレンド2
//法線有り
//テクスチャ有り
**********************************************************/
class RenderObjectB2NX : public RenderObject<VERTEX_B2NX> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_WEIGHT1
	RO_IMPL_INDICES
	RO_IMPL_NORMAL3
	RO_IMPL_UV

	virtual void CalculateWeightCenter();
	void SetVertexBlend(size_t index, size_t pos, uint8_t indexBlend, float rate);
	
protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/b2nx"; }
};

/**********************************************************
//RenderObjectB4NX
//頂点ブレンド4
//法線有り
//テクスチャ有り
**********************************************************/
class RenderObjectB4NX : public RenderObject<VERTEX_B4NX> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_WEIGHT3
	RO_IMPL_INDICES
	RO_IMPL_NORMAL3
	RO_IMPL_UV

	RenderObjectB4NX();
	~RenderObjectB4NX();

	virtual void CalculateWeightCenter();
	void SetVertexBlend(size_t index, size_t pos, uint8_t indexBlend, float rate);
		
protected:
	const char* _GetDefaultShaderName() const override { return "renderobject/b4nx"; }
};

/**********************************************************
//RenderObjectBNX
//頂点ブレンド
//法線有り
//テクスチャ有り
**********************************************************/
class RenderObjectBNX : public RenderObject<VERTEX_BNX> {
public:
	RO_IMPL_VERTEX3
	RO_IMPL_NORMAL3
	RO_IMPL_UV

	RenderObjectBNX();


	virtual void InitializeVertexBuffer(); // ?
	virtual void Render(); // -> Submit

	//描画用設定
	/*void SetMatrix(gstd::ref_count_ptr<Matrices> matrix) { matrix_ = matrix; } // Matrices ??
	void SetColor(const uint32_t color) { color_ = color; }*/

protected:
	//gstd::ref_count_ptr<Matrices> matrix_;
	glm::vec4 color_;
	//D3DMATERIAL9 materialBNX_;

	//virtual void _CopyVertexBufferOnInitialize() = 0;
	const char* _GetDefaultShaderName() const override { return "renderobject/bnx"; }
};

#if 0
class RenderObjectBNXBlock : public RenderBlock {
public:
	void SetMatrix(gstd::ref_count_ptr<Matrices> matrix) { matrix_ = matrix; }
	void SetColor(D3DCOLOR color) { color_ = color; }
	bool IsTranslucent() { return ColorAccess::GetColorA(color_) != 255; }

protected:
	gstd::ref_count_ptr<Matrices> matrix_;
	D3DCOLOR color_;
};

class RenderObjectB2NXBlock : public RenderObjectBNXBlock {
public:
	RenderObjectB2NXBlock();
	virtual ~RenderObjectB2NXBlock();
	virtual void Render();
};

class RenderObjectB4NXBlock : public RenderObjectBNXBlock {
public:
	RenderObjectB4NXBlock();
	virtual ~RenderObjectB4NXBlock();
	virtual void Render();
};
#endif

/**********************************************************
//Sprite2D
//矩形スプライト
**********************************************************/
class Sprite2D : public RenderObjectTLX {
public:
	Sprite2D();
	~Sprite2D();

	void SetSourceRect(RECT_F& rcSrc);
	void SetDestinationRect(RECT_F& rcDest);
	void SetDestinationCenter();
	void SetVertex(RECT_F& rcSrc, RECT_F& rcDest, uint32_t color = COLOR_ARGB(255, 255, 255, 255));

	RECT_F GetDestinationRect();

	Sprite2D& operator=(Sprite2D o);
};

#if 0
/**********************************************************
//SpriteList2D
//矩形スプライトリスト
**********************************************************/
class SpriteList2D : public RenderObjectTLX {
public:
	SpriteList2D();
	virtual int GetVertexCount();
	virtual void Render();
	void ClearVertexCount()
	{
		countRenderVertex_ = 0;
		bCloseVertexList_ = false;
	}
	void AddVertex();
	void SetSourceRect(RECT_D& rcSrc) { rcSrc_ = rcSrc; }
	void SetDestinationRect(RECT_D& rcDest) { rcDest_ = rcDest; }
	void SetDestinationCenter();
	D3DCOLOR GetColor() { return color_; }
	void SetColor(D3DCOLOR color) { color_ = color; }
	void CloseVertex();

private:
	int countRenderVertex_;
	RECT_D rcSrc_;
	RECT_D rcDest_;
	D3DCOLOR color_;
	bool bCloseVertexList_;
	void _AddVertex(VERTEX_TLX& vertex);
};

/**********************************************************
//Sprite3D
//矩形スプライト
**********************************************************/
class Sprite3D : public RenderObjectLX {
public:
	Sprite3D();
	~Sprite3D();
	void SetSourceRect(RECT_D& rcSrc);
	void SetDestinationRect(RECT_D& rcDest);
	void SetVertex(RECT_D& rcSrc, RECT_D& rcDest, D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255));
	void SetSourceDestRect(RECT_D& rcSrc);
	void SetVertex(RECT_D& rcSrc, D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255));
	void SetBillboardEnable(bool bEnable) { bBillboard_ = bEnable; }

protected:
	bool bBillboard_;
	virtual D3DXMATRIX _CreateWorldTransformMaxtrix();
};

/**********************************************************
//TrajectoryObject3D
//3D軌跡
**********************************************************/
class TrajectoryObject3D : public RenderObjectLX {
public:
	TrajectoryObject3D();
	~TrajectoryObject3D();
	virtual void Work();
	virtual void Render();
	void SetInitialLine(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
	void AddPoint(D3DXMATRIX mat);
	void SetAlphaVariation(int diff) { diffAlpha_ = diff; }
	void SetComplementCount(int count) { countComplement_ = count; }
	void SetColor(D3DCOLOR color) { color_ = color; }

private:
	struct Data {
		int alpha;
		D3DXVECTOR3 pos1;
		D3DXVECTOR3 pos2;
	};

protected:
	D3DCOLOR color_;
	int diffAlpha_;
	int countComplement_;
	Data dataInit_;
	Data dataLast1_;
	Data dataLast2_;
	std::list<Data> listData_;
	virtual D3DXMATRIX _CreateWorldTransformMaxtrix();

};

/**********************************************************
//DxMesh
**********************************************************/
enum {
	MESH_ELFREINA,
	MESH_METASEQUOIA,
};

class DxMeshManager;
class DxMeshData {
public:
	friend DxMeshManager;

public:
	DxMeshData();
	virtual ~DxMeshData();
	void SetName(std::string name) { name_ = name; }
	std::string& GetName() { return name_; }
	virtual bool CreateFromFileReader(std::shared_ptr<gstd::FileReader> reader) = 0;

protected:
	std::string name_;
	DxMeshManager* manager_;
	volatile bool bLoad_;
};
class DxMesh : public gstd::FileManager::LoadObject {
public:
	friend DxMeshManager;

public:
	DxMesh();
	virtual ~DxMesh();
	virtual void Release();
	bool CreateFromFile(std::string path);
	virtual bool CreateFromFileReader(std::shared_ptr<gstd::FileReader> reader) = 0;
	virtual bool CreateFromFileInLoadThread(std::string path, int type);
	virtual bool CreateFromFileInLoadThread(std::string path) = 0;
	virtual std::string GetPath() = 0;

	virtual void Render() = 0;
	virtual void Render(std::string nameAnime, int time) { Render(); }
	void SetPosition(D3DXVECTOR3 pos) { position_ = pos; }
	void SetPosition(float x, float y, float z)
	{
		position_.x = x;
		position_.y = y;
		position_.z = z;
	}
	void SetX(float x) { position_.x = x; }
	void SetY(float y) { position_.y = y; }
	void SetZ(float z) { position_.z = z; }
	void SetAngle(D3DXVECTOR3 angle) { angle_ = angle; }
	void SetAngleXYZ(float angx = 0.0f, float angy = 0.0f, float angz = 0.0f)
	{
		angle_.x = angx;
		angle_.y = angy;
		angle_.z = angz;
	}
	void SetScale(D3DXVECTOR3 scale) { scale_ = scale; }
	void SetScaleXYZ(float sx = 1.0f, float sy = 1.0f, float sz = 1.0f)
	{
		scale_.x = sx;
		scale_.y = sy;
		scale_.z = sz;
	}

	void SetColor(D3DCOLOR color) { color_ = color; }
	void SetColorRGB(D3DCOLOR color);
	void SetAlpha(int alpha);

	bool IsCoordinate2D() { return bCoordinate2D_; }
	void SetCoordinate2D(bool b) { bCoordinate2D_ = b; }

	std::shared_ptr<RenderBlocks> CreateRenderBlocks() { return NULL; }
	virtual D3DXMATRIX GetAnimationMatrix(std::string nameAnime, double time, std::string nameBone)
	{
		D3DXMATRIX mat;
		D3DXMatrixIdentity(&mat);
		return mat;
	}
	std::shared_ptr<Shader> GetShader() { return shader_; }
	void SetShader(std::shared_ptr<Shader> shader) { shader_ = shader; }

protected:
	D3DXVECTOR3 position_; //移動先座標
	D3DXVECTOR3 angle_; //回転角度
	D3DXVECTOR3 scale_; //拡大率
	D3DCOLOR color_;
	bool bCoordinate2D_; //2D座標指定
	std::shared_ptr<Shader> shader_;

	std::shared_ptr<DxMeshData> data_;
	std::shared_ptr<DxMeshData> _GetFromManager(std::string name);
	void _AddManager(std::string name, std::shared_ptr<DxMeshData> data);
};

/**********************************************************
//DxMeshManager
**********************************************************/
class DxMeshInfoPanel;
class DxMeshManager : public gstd::FileManager::LoadThreadListener {
	friend DxMeshData;
	friend DxMesh;
	friend DxMeshInfoPanel;

public:
	DxMeshManager();
	virtual ~DxMeshManager();
	static DxMeshManager* GetBase() { return thisBase_; }
	bool Initialize();
	gstd::CriticalSection& GetLock() { return lock_; }

	virtual void Clear();
	virtual void Add(std::string name, std::shared_ptr<DxMesh> mesh); //参照を保持します
	virtual void Release(std::string name); //保持している参照を解放します
	virtual bool IsDataExists(std::string name);

	std::shared_ptr<DxMesh> CreateFromFileInLoadThread(std::string path, int type);
	virtual void CallFromLoadThread(std::unique_ptr<gstd::FileManager::LoadThreadEvent>& event);

	void SetInfoPanel(std::shared_ptr<DxMeshInfoPanel> panel) { panelInfo_ = panel; }

protected:
	gstd::CriticalSection lock_;
	std::map<std::string, std::shared_ptr<DxMesh>> mapMesh_;
	std::map<std::string, std::shared_ptr<DxMeshData>> mapMeshData_;
	std::shared_ptr<DxMeshInfoPanel> panelInfo_;

	void _AddMeshData(std::string name, std::shared_ptr<DxMeshData> data);
	std::shared_ptr<DxMeshData> _GetMeshData(std::string name);
	void _ReleaseMeshData(std::string name);

private:
	static DxMeshManager* thisBase_;
};
#endif
	
} // namespace directx

#endif
