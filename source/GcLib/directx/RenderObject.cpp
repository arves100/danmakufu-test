#include "RenderObject.hpp"

#include "DirectGraphics.hpp"
#include "Shader.hpp"

#if 0
#include "ElfreinaMesh.hpp"
#include "MetasequoiaMesh.hpp"
#endif

using namespace gstd;
using namespace directx;

#if 0
/**********************************************************
//RenderBlock
**********************************************************/
RenderBlock::RenderBlock()
{
	posSortKey_ = 0;
}
RenderBlock::~RenderBlock()
{
	func_ = NULL;
	obj_ = NULL;
}
void RenderBlock::Render()
{
	obj_->SetPosition(position_);
	obj_->SetAngle(angle_);
	obj_->SetScale(scale_);
	if (func_ != NULL)
		func_->CallRenderStateFunction();
	obj_->Render();
}

/**********************************************************
//RenderManager
**********************************************************/
RenderManager::RenderManager()
{
}
RenderManager::~RenderManager()
{
}
void RenderManager::Render()
{
	DirectGraphics* graph = DirectGraphics::GetBase();

	//不透明
	graph->SetZBufferEnable(true);
	graph->SetZWriteEnalbe(true);
	std::list<std::shared_ptr<RenderBlock>>::iterator itrOpaque;
	for (itrOpaque = listBlockOpaque_.begin(); itrOpaque != listBlockOpaque_.end(); itrOpaque++) {
		(*itrOpaque)->Render();
	}

	//半透明
	graph->SetZBufferEnable(true);
	graph->SetZWriteEnalbe(false);
	std::list<std::shared_ptr<RenderBlock>>::iterator itrTrans;
	for (itrTrans = listBlockTranslucent_.begin(); itrTrans != listBlockTranslucent_.end(); itrTrans++) {
		(*itrTrans)->CalculateZValue();
	}
	SortUtility::CombSort(listBlockTranslucent_.begin(), listBlockTranslucent_.end(), ComparatorRenderBlockTranslucent());
	for (itrTrans = listBlockTranslucent_.begin(); itrTrans != listBlockTranslucent_.end(); itrTrans++) {
		(*itrTrans)->Render();
	}

	listBlockOpaque_.clear();
	listBlockTranslucent_.clear();
}
void RenderManager::AddBlock(std::shared_ptr<RenderBlock> block)
{
	if (block == NULL)
		return;
	if (block->IsTranslucent()) {
		listBlockTranslucent_.push_back(block);
	} else {
		listBlockOpaque_.push_back(block);
	}
}
void RenderManager::AddBlock(std::shared_ptr<RenderBlocks> blocks)
{
	std::list<std::shared_ptr<RenderBlock>>& listBlock = blocks->GetList();
	int size = listBlock.size();
	std::list<std::shared_ptr<RenderBlock>>::iterator itr;
	for (itr = listBlock.begin(); itr != listBlock.end(); itr++) {
		AddBlock(*itr);
	}
}

/**********************************************************
//RenderStateFunction
**********************************************************/
RenderStateFunction::RenderStateFunction()
{
}
RenderStateFunction::~RenderStateFunction()
{
}
void RenderStateFunction::CallRenderStateFunction()
{
	std::map<RenderStateFunction::FUNC_TYPE, std::shared_ptr<gstd::ByteBuffer>>::iterator itr;
	for (itr = mapFuncRenderState_.begin(); itr != mapFuncRenderState_.end(); itr++) {
		RenderStateFunction::FUNC_TYPE type = (*itr).first;
		auto args = (*itr).second;
		args->Seek(0);
		if (type == RenderStateFunction::FUNC_LIGHTING) {
			bool bEnable = args->ReadBoolean();
			DirectGraphics::GetBase()->SetLightingEnable(bEnable);
		}
		//TODO
	}
	mapFuncRenderState_.clear();
}

void RenderStateFunction::SetLightingEnable(bool bEnable)
{
	int sizeArgs = sizeof(bEnable);
	auto args = std::make_shared<gstd::ByteBuffer>();
	args->SetSize(sizeArgs);
	args->WriteBoolean(bEnable);
	mapFuncRenderState_[RenderStateFunction::FUNC_LIGHTING] = args;
}
void RenderStateFunction::SetCullingMode(DWORD mode)
{
	int sizeArgs = sizeof(mode);
	auto args = std::make_shared<gstd::ByteBuffer>();
	args->SetSize(sizeArgs);
	args->WriteInteger(mode);
	mapFuncRenderState_[RenderStateFunction::FUNC_CULLING] = args;
}
void RenderStateFunction::SetZBufferEnable(bool bEnable)
{
	int sizeArgs = sizeof(bEnable);
	auto args = std::make_shared<gstd::ByteBuffer>();
	args->SetSize(sizeArgs);
	args->WriteBoolean(bEnable);
	mapFuncRenderState_[RenderStateFunction::FUNC_ZBUFFER_ENABLE] = args;
}
void RenderStateFunction::SetZWriteEnalbe(bool bEnable)
{
	int sizeArgs = sizeof(bEnable);
	auto args = std::make_shared<gstd::ByteBuffer>();
	args->SetSize(sizeArgs);
	args->WriteBoolean(bEnable);
	mapFuncRenderState_[RenderStateFunction::FUNC_ZBUFFER_WRITE_ENABLE] = args;
}
void RenderStateFunction::SetBlendMode(DWORD mode, int stage)
{
	int sizeArgs = sizeof(mode) + sizeof(stage);
	auto args = std::make_shared<gstd::ByteBuffer>();
	args->SetSize(sizeArgs);
	args->WriteInteger(mode);
	args->WriteInteger(stage);
	mapFuncRenderState_[RenderStateFunction::FUNC_BLEND] = args;
}
void RenderStateFunction::SetTextureFilter(DWORD mode, int stage)
{
	int sizeArgs = sizeof(mode) + sizeof(stage);
	auto args = std::make_shared<gstd::ByteBuffer>();
	args->SetSize(sizeArgs);
	args->WriteInteger(mode);
	args->WriteInteger(stage);
	mapFuncRenderState_[RenderStateFunction::FUNC_TEXTURE_FILTER] = args;
}
#endif

/**********************************************************
//RenderObjectTLX
//座標3D変換済み、ライティング済み、テクスチャ有り
**********************************************************/
RenderObjectTLX::RenderObjectTLX()
{
	_SetTextureStageCount(1);
	bPermitCamera_ = true;
}

void RenderObjectTLX::Submit(bgfx::ViewId id)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	DxCamera2D* camera = graphics->GetCamera2D();

	//座標変換
	bool bPos = position_.x != 0.0f || position_.y != 0.0f || position_.z != 0.0f;
	bool bAngle = angle_.x != 0.0f || angle_.y != 0.0f || angle_.z != 0.0f;
	bool bScale = scale_.x != 1.0f || scale_.y != 1.0f || scale_.z != 1.0f;
	bool bCamera = camera->IsEnable() && bPermitCamera_;

	glm::mat4 mat = glm::identity<glm::mat4>();

	if (bPos || bAngle || bScale || bCamera) {
		//座標変換有りなら、座標3D変換済みなため自前で変換をかける
		if (bScale) {
			glm::mat4 matScale;
			matScale = glm::scale(matScale, glm::vec3(scale_.x, scale_.y, scale_.z));
			mat = mat * matScale;
		}
		if (bAngle) {
			glm::mat4 matRot = glm::yawPitchRoll(glm::radians(angle_.y), glm::radians(angle_.x), glm::radians(angle_.z));
			mat = mat * matRot;
		}
		if (bPos) {
			glm::mat4 matTrans;
			matTrans = glm::translate(matTrans, glm::vec3(position_.x, position_.y, position_.z));
			mat = mat * matTrans;
		}

		if (bCamera) {
			auto matCamera = camera->GetMatrix();
			mat = mat * matCamera;
		}
	}

	// what should be done: vertex->position = vertex->position * u_camera_matrix
	shader_->AddParameter("u_camera_matrix", bgfx::UniformType::Mat4, &mat[0], sizeof(mat));
	RenderObject::Submit(id);
}

void RenderObjectTLX::SetVertexCount(const size_t count)
{
	RenderObject::SetVertexCount(count);
	SetColorRGB(COLOR_ARGB(255, 255, 255, 255));
	SetAlpha(255);
}

/**********************************************************
//RenderObjectLX
//ライティング済み、テクスチャ有り
**********************************************************/
RenderObjectLX::RenderObjectLX()
{
	_SetTextureStageCount(1);

}

void RenderObjectLX::SetVertexCount(const size_t count)
{
	RenderObject::SetVertexCount(count);
	SetColorRGB(COLOR_ARGB(255, 255, 255, 255));
	SetAlpha(255);
}


/**********************************************************
//RenderObjectNX
**********************************************************/
RenderObjectNX::RenderObjectNX()
{
	_SetTextureStageCount(1);

}

void RenderObjectNX::Submit(bgfx::ViewId id)
{
	// shader submit

	bool bFogEnable = false;
	glm::mat4 matView;
	glm::mat4 matProj;
	auto device = DirectGraphics::GetBase();

	if (bCoordinate2D_) {
		matProj = device->GetProjMatrix();
		matView = device->GetViewMatrix();
		bFogEnable = device->IsFogEnable();
		device->SetFogEnable(false);
		_SetCoordinate2dDeviceMatrix();
	}

	auto mat = _CreateWorldTransformMatrix();

	// world matrix -> vs
	shader_->AddParameter("u_world_matrix", bgfx::UniformType::Mat4, &mat[0], sizeof(mat));

	RenderObject::Submit(id);

	if (bCoordinate2D_) {
		device->SetViewAndProjMatrix(matView, matProj);
		device->SetFogEnable(bFogEnable);
	}
}

/**********************************************************
//RenderObjectBNX
**********************************************************/
RenderObjectBNX::RenderObjectBNX()
{
	_SetTextureStageCount(1);
	color_ = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	/*materialBNX_.Diffuse.a = 1.0f;
	materialBNX_.Diffuse.r = 1.0f;
	materialBNX_.Diffuse.g = 1.0f;
	materialBNX_.Diffuse.b = 1.0f;
	materialBNX_.Ambient.a = 1.0f;
	materialBNX_.Ambient.r = 1.0f;
	materialBNX_.Ambient.g = 1.0f;
	materialBNX_.Ambient.b = 1.0f;
	materialBNX_.Specular.a = 1.0f;
	materialBNX_.Specular.r = 1.0f;
	materialBNX_.Specular.g = 1.0f;
	materialBNX_.Specular.b = 1.0f;
	materialBNX_.Emissive.a = 1.0f;
	materialBNX_.Emissive.r = 1.0f;
	materialBNX_.Emissive.g = 1.0f;
	materialBNX_.Emissive.b = 1.0f;*/
}

void RenderObjectBNX::Render()
{
	/*IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
	ref_count_ptr<Texture>& texture = texture_[0];
	if (texture != NULL)
		device->SetTexture(0, texture->GetD3DTexture());
	else
		device->SetTexture(0, NULL);

	DWORD bFogEnable = FALSE;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
	if (bCoordinate2D_) {
		device->GetTransform(D3DTS_VIEW, &matView);
		device->GetTransform(D3DTS_PROJECTION, &matProj);
		device->GetRenderState(D3DRS_FOGENABLE, &bFogEnable);
		device->SetRenderState(D3DRS_FOGENABLE, FALSE);
		_SetCoordinate2dDeviceMatrix();
	}

	if (false) {
		D3DXMATRIX mat = _CreateWorldTransformMaxtrix();
		device->SetTransform(D3DTS_WORLD, &mat);

		int sizeMatrix = matrix_->GetSize();
		for (int iMatrix = 0; iMatrix < sizeMatrix; iMatrix++) {
			D3DXMATRIX matrix = matrix_->GetMatrix(iMatrix) * mat;
			device->SetTransform(D3DTS_WORLDMATRIX(iMatrix),
				&matrix);
		}

		device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);

		device->SetFVF(VERTEX_B2NX::fvf);
		if (vertexIndices_.size() == 0) {
			device->DrawPrimitiveUP(typePrimitive_, _GetPrimitiveCount(), vertex_.GetPointer(), strideVertexStreamZero_);
		} else {
			device->DrawIndexedPrimitiveUP(typePrimitive_, 0,
				GetVertexCount(), _GetPrimitiveCount(),
				&vertexIndices_[0], D3DFMT_INDEX16,
				vertex_.GetPointer(), strideVertexStreamZero_);
		}

		device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	} else {
		ShaderManager* manager = ShaderManager::GetBase();
		std::shared_ptr<Shader> shader = manager->GetDefaultSkinnedMeshShader();
		if (shader != NULL) {
			shader->SetTechnique("BasicTec");

			D3DXMATRIX matView;
			device->GetTransform(D3DTS_VIEW, &matView);
			D3DXMATRIX matProj;
			device->GetTransform(D3DTS_PROJECTION, &matProj);

			auto mVP = matView * matProj;
			shader->SetMatrix("mViewProj", mVP);

			D3DXMATRIX matWorld = _CreateWorldTransformMaxtrix();
			D3DLIGHT9 light;
			device->GetLight(0, &light);
			D3DCOLORVALUE diffuse = materialBNX_.Diffuse;
			diffuse.r = _MIN(diffuse.r + light.Diffuse.r, 1.0f);
			diffuse.g = _MIN(diffuse.g + light.Diffuse.g, 1.0f);
			diffuse.b = _MIN(diffuse.b + light.Diffuse.b, 1.0f);
			diffuse = ColorAccess::SetColor(diffuse, color_);

			D3DCOLORVALUE ambient = materialBNX_.Ambient;
			ambient.r = _MIN(ambient.r + light.Ambient.r, 1.0f);
			ambient.g = _MIN(ambient.g + light.Ambient.g, 1.0f);
			ambient.b = _MIN(ambient.b + light.Ambient.b, 1.0f);
			ambient = ColorAccess::SetColor(ambient, color_);

			//ライト
			auto v = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			shader->SetVector("lightDirection", v);
			shader->SetVector("lightDiffuse", v);
			shader->SetVector("materialDiffuse", (D3DXVECTOR4&)diffuse);
			shader->SetVector("materialAmbient", (D3DXVECTOR4&)ambient);

			//フォグ
			DWORD fogNear = 0;
			DWORD fogFar = 0;
			device->GetRenderState(D3DRS_FOGSTART, &fogNear);
			device->GetRenderState(D3DRS_FOGEND, &fogFar);
			shader->SetFloat("fogNear", (float&)fogNear);
			shader->SetFloat("fogFar", (float&)fogFar);

			//座標変換
			int sizeMatrix = matrix_->GetSize();
			std::vector<D3DXMATRIX> listMatrix(sizeMatrix);
			for (int iMatrix = 0; iMatrix < sizeMatrix; iMatrix++) {
				D3DXMATRIX matrix = matrix_->GetMatrix(iMatrix) * matWorld;
				listMatrix[iMatrix] = matrix;
			}
			shader->SetMatrixArray("mWorldMatrixArray", listMatrix);

			unsigned int numPass = shader->Begin();

			device->SetVertexDeclaration(pVertexDecl_);
			device->SetStreamSource(0, pVertexBuffer_, 0, sizeof(Vertex));
			if (vertexIndices_.size() == 0) {
				device->SetIndices(NULL);
				device->DrawPrimitive(typePrimitive_, 0, _GetPrimitiveCount());
			} else {
				device->SetIndices(pIndexBuffer_);
				device->DrawIndexedPrimitive(typePrimitive_, 0,
					0, GetVertexCount(),
					0, _GetPrimitiveCount());
			}

			shader->End();
		} else {
			Logger::WriteTop(StringUtility::Format(u8"Shader error[%s]", "DEFAULT"));
		}

		device->SetVertexDeclaration(NULL);
		device->SetIndices(NULL);
		device->SetTexture(0, NULL);
	}

	if (bCoordinate2D_) {
		device->SetTransform(D3DTS_VIEW, &matView);
		device->SetTransform(D3DTS_PROJECTION, &matProj);
		device->SetRenderState(D3DRS_FOGENABLE, bFogEnable);
	}*/
}

void RenderObjectBNX::InitializeVertexBuffer()
{}

/**********************************************************
//RenderObjectB2NX
**********************************************************/
/*void RenderObjectB2NX::_CopyVertexBufferOnInitialize()
{
	int countVertex = GetVertexCount();
	Vertex* bufVertex;
	HRESULT hrLockVertex = pVertexBuffer_->Lock(0, 0, reinterpret_cast<void**>(&bufVertex), D3DLOCK_NOSYSLOCK);
	if (!FAILED(hrLockVertex)) {
		for (int iVert = 0; iVert < countVertex; iVert++) {
			Vertex* dest = &bufVertex[iVert];
			VERTEX_B2NX* src = GetVertex(iVert);
			ZeroMemory(dest, sizeof(Vertex));

			dest->position = src->position;
			dest->normal = src->normal;
			dest->texcoord = src->texcoord;

			for (int iBlend = 0; iBlend < 2; iBlend++) {
				int indexBlend = BitAccess::GetByte(src->blendIndex, iBlend * 8);
				dest->blendIndex[iBlend] = indexBlend;
			}

			dest->blendRate[0] = src->blendRate;
			dest->blendRate[1] = 1.0f - dest->blendRate[0];
		}
		pVertexBuffer_->Unlock();
	}
}*/

void RenderObjectB2NX::CalculateWeightCenter()
{
	double xTotal = 0;
	double yTotal = 0;
	double zTotal = 0;
	auto countVert = GetVertexCount();
	for (auto iVert = 0; iVert < countVert; iVert++) {
		VERTEX_B2NX* vertex = GetVertex(iVert);
		xTotal += vertex->x;
		yTotal += vertex->y;
		zTotal += vertex->z;
	}
	xTotal /= countVert;
	yTotal /= countVert;
	zTotal /= countVert;
	posWeightCenter_.x = static_cast<float>(xTotal);
	posWeightCenter_.y = static_cast<float>(yTotal);
	posWeightCenter_.z = static_cast<float>(zTotal);
}

void RenderObjectB2NX::SetVertexBlend(size_t index, size_t pos, uint8_t indexBlend, float rate)
{
	VERTEX_B2NX* vertex = GetVertex(index);
	if (vertex == nullptr)
		return;

	BitAccess::SetByte(vertex->indices, pos * 8, indexBlend);
	if (pos == 0)
		vertex->weight = rate;
}

#if 0
//RenderObjectB2NXBlock
RenderObjectB2NXBlock::RenderObjectB2NXBlock()
{
}
RenderObjectB2NXBlock::~RenderObjectB2NXBlock()
{
}
void RenderObjectB2NXBlock::Render()
{
	auto obj = std::dynamic_pointer_cast<RenderObjectB2NX>(obj_);
	obj->SetMatrix(matrix_);
	RenderBlock::Render();
}
#endif

/**********************************************************
//RenderObjectB4NX
**********************************************************/
RenderObjectB4NX::RenderObjectB4NX()
{

}
RenderObjectB4NX::~RenderObjectB4NX()
{
}

#if 0
void RenderObjectB4NX::_CopyVertexBufferOnInitialize()
{
	int countVertex = GetVertexCount();
	Vertex* bufVertex;
	HRESULT hrLockVertex = pVertexBuffer_->Lock(0, 0, reinterpret_cast<void**>(&bufVertex), D3DLOCK_NOSYSLOCK);
	if (!FAILED(hrLockVertex)) {
		for (int iVert = 0; iVert < countVertex; iVert++) {
			Vertex* dest = &bufVertex[iVert];
			VERTEX_B4NX* src = GetVertex(iVert);
			ZeroMemory(dest, sizeof(Vertex));

			dest->position = src->position;
			dest->normal = src->normal;
			dest->texcoord = src->texcoord;

			for (int iBlend = 0; iBlend < 4; iBlend++) {
				int indexBlend = BitAccess::GetByte(src->blendIndex, iBlend * 8);
				dest->blendIndex[iBlend] = indexBlend;
			}

			float lastRate = 1.0f;
			for (int iRate = 0; iRate < 3; iRate++) {
				float rate = src->blendRate[iRate];
				dest->blendRate[iRate] = rate;
				lastRate -= rate;
			}
			dest->blendRate[3] = lastRate;
		}
		pVertexBuffer_->Unlock();
	}
}
#endif

void RenderObjectB4NX::CalculateWeightCenter()
{
	double xTotal = 0;
	double yTotal = 0;
	double zTotal = 0;
	auto countVert = GetVertexCount();
	for (auto iVert = 0; iVert < countVert; iVert++) {
		auto vertex = GetVertex(iVert);
		xTotal += vertex->x;
		yTotal += vertex->y;
		zTotal += vertex->z;
	}
	xTotal /= countVert;
	yTotal /= countVert;
	zTotal /= countVert;
	posWeightCenter_.x = static_cast<float>(xTotal);
	posWeightCenter_.y = static_cast<float>(yTotal);
	posWeightCenter_.z = static_cast<float>(zTotal);
}

void RenderObjectB4NX::SetVertexBlend(size_t index, size_t pos, uint8_t indexBlend, float rate)
{
	VERTEX_B4NX* vertex = GetVertex(index);
	if (!vertex)
		return;
	BitAccess::SetByte(vertex->indices, pos * 8, indexBlend);
	if (pos <= 2)
		vertex->weight[pos] = rate;
}

#if 0
//RenderObjectB4NXBlock
RenderObjectB4NXBlock::RenderObjectB4NXBlock()
{
}
RenderObjectB4NXBlock::~RenderObjectB4NXBlock()
{
}
void RenderObjectB4NXBlock::Render()
{
	auto obj = std::dynamic_pointer_cast<RenderObjectB4NX>(obj_);
	obj->SetMatrix(matrix_);
	RenderBlock::Render();
}
#endif

/**********************************************************
//Sprite2D
//矩形スプライト
**********************************************************/
Sprite2D::Sprite2D()
{
	SetVertexCount(4); //左上、右上、左下、右下
	SetIndicesCount(6);
	SetIndex(0, 0);
	SetIndex(1, 1);
	SetIndex(2, 2);
	SetIndex(3, 2);
	SetIndex(4, 3);
	SetIndex(5, 1);
	Finalize();
}

Sprite2D::~Sprite2D()
{
}

Sprite2D& Sprite2D::operator=(Sprite2D o)
{
	typePrimitive_ = o.typePrimitive_;

	vertex_ = o.vertex_;
	vertexIndices_ = o.vertexIndices_;
	texture_ = o.texture_;

	posWeightCenter_ = o.posWeightCenter_;

	position_ = o.position_;
	angle_ = o.angle_;
	scale_ = o.scale_;
	matRelative_ = o.matRelative_;
	return *this;
}

void Sprite2D::SetSourceRect(RECT_F& rcSrc)
{
	auto& texture = texture_[0]; // diffuse
	if (!texture)
		return;

	auto width = static_cast<float>(texture->GetWidth());
	auto height = static_cast<float>(texture->GetHeight());

	//テクスチャUV
	SetVertexUV(3, rcSrc.right / width, rcSrc.top / height);
	SetVertexUV(2, rcSrc.left / width, rcSrc.top / height);
	SetVertexUV(0, rcSrc.left / width, rcSrc.bottom / height);
	SetVertexUV(1, rcSrc.right / width, rcSrc.bottom / height);

	name_ = texture->GetName();
}

void Sprite2D::SetDestinationRect(RECT_F& rcDest)
{
	//頂点位置
	SetVertexPosition(0, rcDest.left, rcDest.top);
	SetVertexPosition(1, rcDest.right, rcDest.top);
	SetVertexPosition(2, rcDest.left, rcDest.bottom);
	SetVertexPosition(3, rcDest.right, rcDest.bottom);
}

void Sprite2D::SetDestinationCenter()
{
	auto& texture = texture_[0];
	if (texture == nullptr || GetVertexCount() < 4)
		return;
	auto width = texture->GetWidth();
	auto height = texture->GetHeight();

	auto vertLT = GetVertex(0); //左上
	auto vertRB = GetVertex(3); //右下

	auto vWidth = vertRB->u * width - vertLT->u * width;
	auto vHeight = vertRB->v * height - vertLT->v * height;
	RECT_F rcDest = { -vWidth / 2.0f, -vHeight / 2.0f, vWidth / 2.0f, vHeight / 2.0f };

	SetDestinationRect(rcDest);
}

void Sprite2D::SetVertex(RECT_F& rcSrc, RECT_F& rcDest, uint32_t color)
{
	SetSourceRect(rcSrc);
	SetDestinationRect(rcDest);
	SetColorRGB(color);
	SetAlpha(ColorAccess::GetColorA(color));
}

RECT_F Sprite2D::GetDestinationRect()
{
	VERTEX_TLX* vertexLeftTop = GetVertex(0);
	VERTEX_TLX* vertexRightBottom = GetVertex(3);

	return { vertexLeftTop->x - posBias_, vertexLeftTop->y - posBias_, vertexRightBottom->x - posBias_, vertexRightBottom->y - posBias_ };
}

#if 0
/**********************************************************
//SpriteList2D
**********************************************************/
SpriteList2D::SpriteList2D()
{
	countRenderVertex_ = 0;
	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	bCloseVertexList_ = false;
}
void SpriteList2D::Render()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	auto camera = graphics->GetCamera2D();
	IDirect3DDevice9* device = graphics->GetDevice();
	std::shared_ptr<Texture>& texture = texture_[0];
	if (texture != NULL)
		device->SetTexture(0, texture->GetD3DTexture());
	else
		device->SetTexture(0, NULL);
	device->SetFVF(VERTEX_TLX::fvf);

	//座標変換
	bool bPos = position_.x != 0.0f || position_.y != 0.0f || position_.z != 0.0f;
	bool bAngle = angle_.x != 0.0f || angle_.y != 0.0f || angle_.z != 0.0f;
	bool bScale = scale_.x != 1.0f || scale_.y != 1.0f || scale_.z != 1.0f;
	bool bCamera = camera->IsEnable() && bPermitCamera_;
	if (bPos || bAngle || bScale || bCamera) {
		//座標変換有りなら、座標3D変換済みなため自前で変換をかける
		D3DXMATRIX mat;
		D3DXMatrixIdentity(&mat);

		if (bCloseVertexList_) {
			if (bScale) {
				D3DXMATRIX matScale;
				D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
				mat = mat * matScale;
			}
			if (bAngle) {
				D3DXMATRIX matRot;
				D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angle_.z));
				mat = mat * matRot;
			}
			if (bPos) {
				D3DXMATRIX matTrans;
				D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
				mat = mat * matTrans;
			}
		}

		if (bCamera) {
			D3DXMATRIX matCamera = camera->GetMatrix();
			mat = mat * matCamera;
		}

		//頂点情報のコピーをとる
		vertCopy_.Copy(vertex_);

		//各頂点と行列の積を計算する
		int countVertex = GetVertexCount();
		for (int iVert = 0; iVert < countVertex; iVert++) {
			int pos = iVert * strideVertexStreamZero_;
			VERTEX_TLX* vert = (VERTEX_TLX*)vertCopy_.GetPointer(pos);
			D3DXVec3TransformCoord((D3DXVECTOR3*)&vert->position, (D3DXVECTOR3*)&vert->position, &mat);
		}

		//描画
		int oldSamplerState = 0;

		BeginShader();
		if (vertexIndices_.size() == 0) {
			device->DrawPrimitiveUP(typePrimitive_, _GetPrimitiveCount(), vertCopy_.GetPointer(), strideVertexStreamZero_);
		} else {
			device->DrawIndexedPrimitiveUP(typePrimitive_, 0,
				GetVertexCount(), _GetPrimitiveCount(),
				&vertexIndices_[0], D3DFMT_INDEX16,
				vertCopy_.GetPointer(), strideVertexStreamZero_);
		}
		EndShader();

		//頂点情報を元に戻す
		// memcpy(vertex_.GetPointer(), vertCopy_.GetPointer(), vertex_.GetSize());
	} else {
		//座標変換無しならそのまま描画
		BeginShader();
		if (vertexIndices_.size() == 0) {
			device->DrawPrimitiveUP(typePrimitive_, _GetPrimitiveCount(), vertex_.GetPointer(), strideVertexStreamZero_);
		} else {
			device->DrawIndexedPrimitiveUP(typePrimitive_, 0,
				GetVertexCount(), _GetPrimitiveCount(),
				&vertexIndices_[0], D3DFMT_INDEX16,
				vertex_.GetPointer(), strideVertexStreamZero_);
		}
		EndShader();
	}
}
int SpriteList2D::GetVertexCount()
{
	int res = countRenderVertex_;
	res = _MIN(countRenderVertex_, vertex_.GetSize() / strideVertexStreamZero_);
	return res;
}
void SpriteList2D::_AddVertex(VERTEX_TLX& vertex)
{
	int count = vertex_.GetSize() / strideVertexStreamZero_;
	if (countRenderVertex_ >= count) {
		//リサイズ
		int newCount = _MAX(10, count * 1.5);
		ByteBuffer buffer(vertex_);
		SetVertexCount(newCount);
		memcpy(vertex_.GetPointer(), buffer.GetPointer(), buffer.GetSize());
	}
	SetVertex(countRenderVertex_, vertex);
	countRenderVertex_++;
}
void SpriteList2D::AddVertex()
{
	if (bCloseVertexList_)
		return;

	std::shared_ptr<Texture>& texture = texture_[0];
	if (texture == NULL)
		return;

	int width = texture->GetWidth();
	int height = texture->GetHeight();

	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);

	DirectGraphics* graphics = DirectGraphics::GetBase();
	bool bPos = position_.x != 0.0f || position_.y != 0.0f || position_.z != 0.0f;
	bool bAngle = angle_.x != 0.0f || angle_.y != 0.0f || angle_.z != 0.0f;
	bool bScale = scale_.x != 1.0f || scale_.y != 1.0f || scale_.z != 1.0f;

	if (bScale) {
		D3DXMATRIX matScale;
		D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
		mat = mat * matScale;
	}
	if (bAngle) {
		D3DXMATRIX matRot;
		D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angle_.z));
		mat = mat * matRot;
	}
	if (bPos) {
		D3DXMATRIX matTrans;
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		mat = mat * matTrans;
	}

	VERTEX_TLX verts[4];
	float srcX[] = { (float)rcSrc_.left, (float)rcSrc_.right, (float)rcSrc_.left, (float)rcSrc_.right };
	float srcY[] = { (float)rcSrc_.top, (float)rcSrc_.top, (float)rcSrc_.bottom, (float)rcSrc_.bottom };
	int destX[] = { (int)rcDest_.left, (int)rcDest_.right, (int)rcDest_.left, (int)rcDest_.right };
	int destY[] = { (int)rcDest_.top, (int)rcDest_.top, (int)rcDest_.bottom, (int)rcDest_.bottom };
	for (int iVert = 0; iVert < 4; iVert++) {
		VERTEX_TLX& vert = verts[iVert];

		vert.texcoord.x = srcX[iVert] / width;
		vert.texcoord.y = srcY[iVert] / height;

		float bias = -0.5f;
		vert.position.x = destX[iVert] + bias;
		vert.position.y = destY[iVert] + bias;
		vert.position.z = 1.0;
		vert.position.w = 1.0;

		vert.diffuse_color = color_;
		vert.position = DxMath::VectMatMulti(verts[iVert].position, mat);
	}

	_AddVertex(verts[0]);
	_AddVertex(verts[2]);
	_AddVertex(verts[1]);
	_AddVertex(verts[1]);
	_AddVertex(verts[2]);
	_AddVertex(verts[3]);
}
void SpriteList2D::SetDestinationCenter()
{
	std::shared_ptr<Texture>& texture = texture_[0];
	if (texture == NULL)
		return;
	int width = texture->GetWidth();
	int height = texture->GetHeight();

	VERTEX_TLX* vertLT = GetVertex(0); //左上
	VERTEX_TLX* vertRB = GetVertex(3); //右下

	int vWidth = rcSrc_.right - rcSrc_.left;
	int vHeight = rcSrc_.bottom - rcSrc_.top;
	RECT_D rcDest = { -vWidth / 2., -vHeight / 2., vWidth / 2., vHeight / 2. };

	SetDestinationRect(rcDest);
}
void SpriteList2D::CloseVertex()
{
	bCloseVertexList_ = true;

	position_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	angle_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	scale_ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

/**********************************************************
//Sprite3D
**********************************************************/
Sprite3D::Sprite3D()
{
	SetVertexCount(4); //左上、右上、左下、右下
	SetPrimitiveType(D3DPT_TRIANGLESTRIP);
	bBillboard_ = false;
}
Sprite3D::~Sprite3D()
{
}
D3DXMATRIX Sprite3D::_CreateWorldTransformMaxtrix()
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	bool bPos = position_.x != 0.0f || position_.y != 0.0f || position_.z != 0.0f;
	bool bAngle = angle_.x != 0.0f || angle_.y != 0.0f || angle_.z != 0.0f;
	bool bScale = scale_.x != 1.0f || scale_.y != 1.0f || scale_.z != 1.0f;
	if (bPos || bAngle || bScale || bBillboard_) {
		if (bScale) {
			D3DXMATRIX matScale;
			D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
			mat = mat * matScale;
		}
		if (bAngle) {
			D3DXMATRIX matRot;
			D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angle_.z));
			mat = mat * matRot;
		}
		if (bBillboard_) {
			DirectGraphics* graph = DirectGraphics::GetBase();
			IDirect3DDevice9* device = graph->GetDevice();
			D3DXMATRIX matView;
			device->GetTransform(D3DTS_VIEW, &matView);

			D3DXMATRIX matInv;
			D3DXMatrixIdentity(&matInv);
			matInv._11 = matView._11;
			matInv._12 = matView._21;
			matInv._13 = matView._31;
			matInv._21 = matView._12;
			matInv._22 = matView._22;
			matInv._23 = matView._32;
			matInv._31 = matView._13;
			matInv._32 = matView._23;
			matInv._33 = matView._33;
			mat = mat * matInv;
		}
		if (bPos) {
			D3DXMATRIX matTrans;
			D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
			mat = mat * matTrans;
		}
	}

	if (bBillboard_) {
		D3DXMATRIX matRelative;
		D3DXMatrixIdentity(&matRelative);
		D3DXVECTOR3 pos;
		D3DXVec3TransformCoord(&pos, &D3DXVECTOR3(0, 0, 0), &matRelative_);
		D3DXMatrixTranslation(&matRelative, pos.x, pos.y, pos.z);
		mat = mat * matRelative;
	} else {
		mat = mat * matRelative_;
	}
	return mat;
}
void Sprite3D::SetSourceRect(RECT_D& rcSrc)
{
	std::shared_ptr<Texture>& texture = texture_[0];
	if (texture == NULL)
		return;
	int width = texture->GetWidth();
	int height = texture->GetHeight();

	//テクスチャUV
	SetVertexUV(0, (float)rcSrc.left / (float)width, (float)rcSrc.top / (float)height);
	SetVertexUV(1, (float)rcSrc.left / (float)width, (float)rcSrc.bottom / (float)height);
	SetVertexUV(2, (float)rcSrc.right / (float)width, (float)rcSrc.top / (float)height);
	SetVertexUV(3, (float)rcSrc.right / (float)width, (float)rcSrc.bottom / (float)height);
}
void Sprite3D::SetDestinationRect(RECT_D& rcDest)
{
	//頂点位置
	SetVertexPosition(0, rcDest.left, rcDest.top, 0);
	SetVertexPosition(1, rcDest.left, rcDest.bottom, 0);
	SetVertexPosition(2, rcDest.right, rcDest.top, 0);
	SetVertexPosition(3, rcDest.right, rcDest.bottom, 0);
}
void Sprite3D::SetVertex(RECT_D& rcSrc, RECT_D& rcDest, D3DCOLOR color)
{
	SetSourceRect(rcSrc);
	SetDestinationRect(rcDest);

	//頂点色
	SetColorRGB(color);
	SetAlpha(ColorAccess::GetColorA(color));
}

void Sprite3D::SetSourceDestRect(RECT_D& rcSrc)
{
	int width = rcSrc.right - rcSrc.left;
	int height = rcSrc.bottom - rcSrc.top;
	RECT_D rcDest;
	SetRectD(&rcDest, -width / 2., -height / 2., width / 2., height / 2.);

	SetSourceRect(rcSrc);
	SetDestinationRect(rcDest);
}
void Sprite3D::SetVertex(RECT_D& rcSrc, D3DCOLOR color)
{
	SetSourceDestRect(rcSrc);

	//頂点色
	SetColorRGB(color);
	SetAlpha(ColorAccess::GetColorA(color));
}

/**********************************************************
//TrajectoryObject3D
**********************************************************/
TrajectoryObject3D::TrajectoryObject3D()
{
	SetPrimitiveType(D3DPT_TRIANGLESTRIP);
	diffAlpha_ = 20;
	countComplement_ = 8;
	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);
}
TrajectoryObject3D::~TrajectoryObject3D()
{
}
D3DXMATRIX TrajectoryObject3D::_CreateWorldTransformMaxtrix()
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	return mat;
}
void TrajectoryObject3D::Work()
{
	std::list<Data>::iterator itr;
	for (itr = listData_.begin(); itr != listData_.end();) {
		Data& data = (*itr);
		data.alpha -= diffAlpha_;
		if (data.alpha < 0)
			itr = listData_.erase(itr);
		else
			itr++;
	}
}
void TrajectoryObject3D::Render()
{
	int size = listData_.size() * 2;
	SetVertexCount(size);

	int width = 1;
	std::shared_ptr<Texture> texture = texture_[0];
	if (texture != NULL) {
		width = texture->GetWidth();
	}

	float dWidth = 1.0 / width / listData_.size();
	int iData = 0;
	std::list<Data>::iterator itr;
	for (itr = listData_.begin(); itr != listData_.end(); itr++, iData++) {
		Data data = (*itr);
		int alpha = data.alpha;
		for (int iPos = 0; iPos < 2; iPos++) {
			int index = iData * 2 + iPos;
			D3DXVECTOR3& pos = iPos == 0 ? data.pos1 : data.pos2;
			float u = dWidth * iData;
			float v = iPos == 0 ? 0 : 1;

			SetVertexPosition(index, pos.x, pos.y, pos.z);
			SetVertexUV(index, u, v);

			float r = ColorAccess::GetColorR(color_) * alpha / 255;
			float g = ColorAccess::GetColorG(color_) * alpha / 255;
			float b = ColorAccess::GetColorB(color_) * alpha / 255;
			SetVertexColorARGB(index, alpha, r, g, b);
		}
	}
	RenderObjectLX::Render();
}
void TrajectoryObject3D::SetInitialLine(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2)
{
	dataInit_.pos1 = pos1;
	dataInit_.pos2 = pos2;
}
void TrajectoryObject3D::AddPoint(D3DXMATRIX mat)
{
	Data data;
	data.alpha = 255;
	data.pos1 = dataInit_.pos1;
	data.pos2 = dataInit_.pos2;
	D3DXVec3TransformCoord((D3DXVECTOR3*)&data.pos1, (D3DXVECTOR3*)&data.pos1, &mat);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&data.pos2, (D3DXVECTOR3*)&data.pos2, &mat);

	if (listData_.size() <= 1) {
		listData_.push_back(data);
		dataLast2_ = dataLast1_;
		dataLast1_ = data;
	} else {
		float cDiff = 1.0 / (countComplement_);
		float diffAlpha = diffAlpha_ / countComplement_;
		for (int iCount = 0; iCount < countComplement_ - 1; iCount++) {
			Data cData;
			float flame = cDiff * (iCount + 1);
			for (int iPos = 0; iPos < 2; iPos++) {
				D3DXVECTOR3& outPos = iPos == 0 ? cData.pos1 : cData.pos2;
				D3DXVECTOR3& cPos = iPos == 0 ? data.pos1 : data.pos2;
				D3DXVECTOR3& lPos1 = iPos == 0 ? dataLast1_.pos1 : dataLast1_.pos2;
				D3DXVECTOR3& lPos2 = iPos == 0 ? dataLast2_.pos1 : dataLast2_.pos2;

				D3DXVECTOR3 vPos1 = lPos1 - lPos2;
				D3DXVECTOR3 vPos2 = lPos2 - cPos + vPos1;

				// D3DXVECTOR3 vPos1 = lPos2 - lPos1;
				// D3DXVECTOR3 vPos2 = lPos2 - cPos + vPos1;

				// D3DXVECTOR3 vPos1 = lPos1 - lPos2;
				// D3DXVECTOR3 vPos2 = cPos - lPos1;

				D3DXVec3Hermite(&outPos, &lPos1, &vPos1, &cPos, &vPos2, flame);
			}

			cData.alpha = 255 - diffAlpha * (countComplement_ - iCount - 1);
			listData_.push_back(cData);
		}
		dataLast2_ = dataLast1_;
		dataLast1_ = data;
	}
}

/**********************************************************
//DxMesh
**********************************************************/
DxMeshData::DxMeshData()
{
	manager_ = DxMeshManager::GetBase();
	bLoad_ = true;
}
DxMeshData::~DxMeshData()
{
}

DxMesh::DxMesh()
{
	position_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	angle_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	scale_ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	bCoordinate2D_ = false;
}
DxMesh::~DxMesh()
{
	Release();
}
std::shared_ptr<DxMeshData> DxMesh::_GetFromManager(std::string name)
{
	return DxMeshManager::GetBase()->_GetMeshData(name);
}
void DxMesh::_AddManager(std::string name, std::shared_ptr<DxMeshData> data)
{
	DxMeshManager::GetBase()->_AddMeshData(name, data);
}
void DxMesh::Release()
{
	{
		DxMeshManager* manager = DxMeshManager::GetBase();
		Lock lock(manager->GetLock());
		if (data_ != NULL) {
			if (manager->IsDataExists(data_->GetName())) {
				int countRef = data_.use_count();
				//自身とDxMeshManager内の数だけになったら削除
				if (countRef == 2) {
					manager->_ReleaseMeshData(data_->GetName());
				}
			}
			data_ = NULL;
		}
	}
}
bool DxMesh::CreateFromFile(std::string path)
{
	try {
		path = PathProperty::GetUnique(path);
		std::shared_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
		if (reader == NULL)
			throw std::exception(u8"ファイルが見つかりません");
		return CreateFromFileReader(reader);
	} catch (std::exception& e) {
		std::string str = StringUtility::Format(u8"DxMesh：メッシュ読み込み失敗[%s]\n\t%s", path.c_str(), e.what());
		Logger::WriteTop(str);
	}
	return false;
}
bool DxMesh::CreateFromFileInLoadThread(std::string path, int type)
{
	bool res = false;
	{
		Lock lock(DxMeshManager::GetBase()->GetLock());
		if (data_ != NULL)
			Release();
		DxMeshManager* manager = DxMeshManager::GetBase();
		std::shared_ptr<DxMesh> mesh = manager->CreateFromFileInLoadThread(path, type);
		if (mesh != NULL) {
			data_ = mesh->data_;
		}
		res = data_ != NULL;
	}

	return res;
}
void DxMesh::SetColorRGB(D3DCOLOR color)
{
	int r = ColorAccess::GetColorR(color);
	int g = ColorAccess::GetColorG(color);
	int b = ColorAccess::GetColorB(color);
	ColorAccess::SetColorR(color_, r);
	ColorAccess::SetColorG(color_, g);
	ColorAccess::SetColorB(color_, b);
}
void DxMesh::SetAlpha(int alpha)
{
	ColorAccess::SetColorA(color_, alpha);
}
/**********************************************************
//DxMeshManager
**********************************************************/
DxMeshManager* DxMeshManager::thisBase_ = NULL;
DxMeshManager::DxMeshManager()
{
}
DxMeshManager::~DxMeshManager()
{
	this->Clear();
	FileManager::GetBase()->RemoveLoadThreadListener(this);
	panelInfo_ = NULL;
	thisBase_ = NULL;
}
bool DxMeshManager::Initialize()
{
	thisBase_ = this;
	FileManager::GetBase()->AddLoadThreadListener(this);
	return true;
}

void DxMeshManager::Clear()
{
	{
		Lock lock(lock_);
		mapMesh_.clear();
		mapMeshData_.clear();
	}
}

void DxMeshManager::_AddMeshData(std::string name, std::shared_ptr<DxMeshData> data)
{
	{
		Lock lock(lock_);
		if (!IsDataExists(name)) {
			mapMeshData_[name] = data;
		}
	}
}
std::shared_ptr<DxMeshData> DxMeshManager::_GetMeshData(std::string name)
{
	std::shared_ptr<DxMeshData> res = NULL;
	{
		Lock lock(lock_);
		if (IsDataExists(name)) {
			res = mapMeshData_[name];
		}
	}
	return res;
}
void DxMeshManager::_ReleaseMeshData(std::string name)
{
	{
		Lock lock(lock_);
		if (IsDataExists(name)) {
			mapMeshData_.erase(name);
			Logger::WriteTop(StringUtility::Format(u8"DxMeshManager：メッシュを解放しました[%s]", name.c_str()));
		}
	}
}
void DxMeshManager::Add(std::string name, std::shared_ptr<DxMesh> mesh)
{
	{
		Lock lock(lock_);
		bool bExist = mapMesh_.find(name) != mapMesh_.end();
		if (!bExist) {
			mapMesh_[name] = mesh;
		}
	}
}
void DxMeshManager::Release(std::string name)
{
	{
		Lock lock(lock_);
		mapMesh_.erase(name);
	}
}
bool DxMeshManager::IsDataExists(std::string name)
{
	bool res = false;
	{
		Lock lock(lock_);
		res = mapMeshData_.find(name) != mapMeshData_.end();
	}
	return res;
}
std::shared_ptr<DxMesh> DxMeshManager::CreateFromFileInLoadThread(std::string path, int type)
{
	std::shared_ptr<DxMesh> res;
	{
		Lock lock(lock_);
		bool bExist = mapMesh_.find(path) != mapMesh_.end();
		if (bExist) {
			res = mapMesh_[path];
		} else {
			if (type == MESH_ELFREINA)
			{
				mapMesh_[path] = std::make_unique<ElfreinaMesh>();
			}
			else if (type == MESH_METASEQUOIA)
				mapMesh_[path] = std::make_unique<MetasequoiaMesh>();

			res = mapMesh_[path];
			if (!IsDataExists(path)) {
				std::shared_ptr<DxMeshData> data;
				if (type == MESH_ELFREINA)
					mapMeshData_[path] = std::make_shared<ElfreinaMeshData>();
				else if (type == MESH_METASEQUOIA)
					mapMeshData_[path] = std::make_shared<MetasequoiaMeshData>();
				data = mapMeshData_[path];
				data->manager_ = this;
				data->name_ = path;
				data->bLoad_ = false;

				std::shared_ptr<FileManager::LoadObject> source = res;
				auto event = std::make_unique<FileManager::LoadThreadEvent>(this, path, res);
				FileManager::GetBase()->AddLoadThreadEvent(event);
			}

			res->data_ = mapMeshData_[path];
		}
	}
	return res;
}
void DxMeshManager::CallFromLoadThread(std::unique_ptr<gstd::FileManager::LoadThreadEvent>& event)
{
	std::string path = event->GetPath();
	{
		Lock lock(lock_);
		auto src = event->GetSource();
		ref_count_ptr<DxMesh> mesh = ref_count_ptr<DxMesh>::DownCast(src);
		if (mesh == NULL)
			return;

		std::shared_ptr<DxMeshData> data = mesh->data_;
		if (data->bLoad_)
			return;

		bool res = false;
		std::shared_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
		if (reader != NULL && reader->Open()) {
			res = data->CreateFromFileReader(reader);
		}
		if (res) {
			Logger::WriteTop(StringUtility::Format(u8"メッシュを読み込みました[%s]", path.c_str()));
		} else {
			Logger::WriteTop(StringUtility::Format(u8"メッシュを読み込み失敗[%s]", path.c_str()));
		}
		data->bLoad_ = true;
	}
}
#endif
