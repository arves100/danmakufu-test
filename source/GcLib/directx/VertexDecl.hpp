#ifndef __DIRECTX_VERTEXDECL__
#define __DIRECTX_VERTEXDECL__

namespace directx {

// Vertex declaration (from d3d9 FVF)
// Conversion reference: https://docs.microsoft.com/en-us/windows/win32/direct3d9/mapping-fvf-codes-to-a-directx-9-declaration
// D3DDECLUSAGE_BLENDWEIGHT -> a_weight (Attrib weight)
// D3DDECLUSAGE_BLENDINDICES -> a_indices (Attrib indices)

// Vertex with Light (L) Texture (T) Tangent (A)
// only used on TestMode.cpp bump program
struct VERTEX_LTA
{
	float x, y, z;
	uint32_t normal, tangent;
	int16_t u, v;

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
		.end();
	}
};

/**********************************************************
//Vertex declarations
**********************************************************/

// Vertex with W Transform (T) Light (L)
struct VERTEX_TL {
	float x, y, z, w;
	uint32_t color; // r g b a

	VERTEX_TL() = default;
	VERTEX_TL(glm::vec4 pos, uint32_t color) : x(pos.x), y(pos.y), z(pos.z), w(pos.w), color(color) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			.end();
	}
};

// Vertex with W Transform (T) Light (L) Texture (X) 
struct VERTEX_TLX {
	float x, y, z, w;
	uint32_t color; // r g b a
	float u, v;

	VERTEX_TLX() = default;
	VERTEX_TLX(glm::vec4 pos, uint32_t color, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), w(pos.w), color(color), u(uv.x), v(uv.y) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with Light (L)
struct VERTEX_L {
	float x, y, z;
	uint32_t color; // r g b a

	VERTEX_L() = default;
	VERTEX_L(glm::vec3 pos, uint32_t color) : x(pos.x), y(pos.y), z(pos.z), color(color) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			.end();
	}
};

// Vertex with Texture (X) and Light (L)
struct VERTEX_LX {
	float x, y, z;
	uint32_t color; // r g b a
	float u, v;

	VERTEX_LX() = default;
	VERTEX_LX(glm::vec3 pos, uint32_t color, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), color(color), u(uv.x), v(uv.y) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with Normal (N)
struct VERTEX_N {
	//未ライティング
	
	float x, y, z;
	float nx, ny, nz;

	VERTEX_N() = default;
	VERTEX_N(glm::vec3 pos, glm::vec3 n) : x(pos.x), y(pos.y), z(pos.z), nx(n.x), ny(n.y), nz(n.z) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with Normal (N) Texture (X)
struct VERTEX_NX {
	//未ライティング、テクスチャ有り

	float x, y, z;
	float nx, ny, nz;
	float u, v;
	
	VERTEX_NX() = default;
	VERTEX_NX(glm::vec3 pos, glm::vec3 n, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), nx(n.x), ny(n.y), nz(n.z), u(uv.x), v(uv.y) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with Normal (N) Texture (X) G?
struct VERTEX_NXG {
	float x, y, z;
	float weight[3]; // blend
	float nx, ny, nz;
	float u, v;

	VERTEX_NXG() = default;
	VERTEX_NXG(glm::vec3 pos, glm::vec3 n, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), nx(n.x), ny(n.y), nz(n.z), u(uv.x), v(uv.y) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float) // a_position
			.add(bgfx::Attrib::Weight, 3, bgfx::AttribType::Float) // a_weight
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float) // a_normal
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float) // a_texcoord1
			.end();
	}
};

// Vertex with one blending value, the blend indices (B1), Normal (N) and Texture (X)
struct VERTEX_B1NX {
	float x, y, z;
	uint32_t indices;
	float nx, ny, nz; // normal
	float u, v;

	VERTEX_B1NX() = default;
	VERTEX_B1NX(glm::vec3 pos, uint32_t bi, glm::vec3 n, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), indices(bi), nx(n.x), ny(n.y), nz(n.z), u(uv.x), v(uv.y) {}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Uint8, true, true) // a_indices
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with two blending value (B2), the blend indices and blend weight, Normal (N) and Texture (X)
struct VERTEX_B2NX {
	float x, y, z;
	float weight; // blend rate
	uint32_t indices; // blend index
	float nx, ny, nz; // normal
	float u, v;

	VERTEX_B2NX() = default;
	VERTEX_B2NX(glm::vec3 pos, float rate, uint8_t index1, uint8_t index2, glm::vec3 n, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), weight(rate), nx(n.x), ny(n.y), nz(n.z), u(uv.x), v(uv.y)
	{
		gstd::BitAccess::SetByte(indices, 0, index1);
		gstd::BitAccess::SetByte(indices, 8, index2);
	}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Weight, 1, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Uint8, true, true) // a_indices
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with four blending value (B4), the blend indices and the three blend weight, Normal (N) and Texture (X)
struct VERTEX_B4NX {
	float x, y, z;
	float weight[3]; // blend rate
	uint32_t indices; // blend index
	float nx, ny, nz; // normal
	float u, v;

	VERTEX_B4NX() = default;
	VERTEX_B4NX(glm::vec3 pos, float rate[3], uint8_t index[4], glm::vec3 n, glm::vec2 uv) : x(pos.x), y(pos.y), z(pos.z), nx(n.x), ny(n.y), nz(n.z), u(uv.x), v(uv.y)
	{
		for (auto iRate = 0; iRate < 3; iRate++)
			weight[iRate] = rate[iRate];
		for (auto iIndex = 0; iIndex < 4; iIndex++)
			gstd::BitAccess::SetByte(indices, 8 * iIndex, index[iIndex]);
	}

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Weight, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Uint8, true, true) // a_indices
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// Vertex with complete blending (B), 4 blend weight and 3 blend indices, Normal (N), Texture (X)
struct VERTEX_BNX {
	float x, y, z;
	float weight[4];
	float indices[3];
	float nx, ny, nz;
	float u, v;
	
	VERTEX_BNX() = default;

	static void get(bgfx::VertexLayout& vl)
	{
		vl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Indices, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
			.end();
	}
};

// --- RenderObject impl code


#define RO_IMPL_VERTEX3 \
	void SetVertexPosition(size_t index, float x, float y, float z = 1.0f) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->x = x + posBias_; \
		vertex->y = y + posBias_; \
		vertex->z = z; \
	}

#define RO_IMPL_VERTEX4 \
	void SetVertexPosition(size_t index, float x, float y, float z = 1.0f, float w = 1.0f) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->x = x + posBias_; \
		vertex->y = y + posBias_; \
		vertex->z = z; \
		vertex->w = w; \
	}

#define RO_IMPL_UV \
	void SetVertexUV(size_t index, float u, float v) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->u = u; \
		vertex->v = v; \
	}

#define RO_IMPL_TANGENT \
	void SetVertexTangent(size_t index, uint8_t x, uint8_t y, uint8_t z, uint8_t w) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->tangent = encodeNormalRgba8(x, y, z, w); \
	} \
	void SetVertexTangent(size_t index, uint32_t n) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->tangent = n; \
	}

#define RO_IMPL_NORMAL4 \
	void SetVertexNormal(size_t index, uint8_t x, uint8_t y, uint8_t z, uint8_t w) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->normal = encodeNormalRgba8(x, y, z, w); \
	} \
	void SetVertexNormal(size_t index, uint32_t n) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->normal = n; \
	}

#define RO_IMPL_NORMAL3 \
	void SetVertexNormal(size_t index, float x, float y, float z) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->nx = x; \
		vertex->ny = y; \
		vertex->nz = z; \
	}

#define RO_IMPL_WEIGHT3 \
	void SetVertexWeight(size_t index, float x, float y, float z) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->weight[0] = x; \
		vertex->weight[1] = y; \
		vertex->weight[2] = z; \
	}

#define RO_IMPL_WEIGHT1 \
	void SetVertexWeight(size_t index, float b) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->weight = b; \
	}

#define RO_IMPL_INDICES \
	void SetVertexIndices(size_t index, uint32_t i) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->indices = i; \
	} \
	void SetVertexIndices(size_t index, uint8_t x, uint8_t y, uint8_t z, uint8_t w) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->indices = COLOR_ARGB(x,y,z,w); \
	}

#define RO_IMPL_RGBA \
	void SetVertexColor(size_t index, uint32_t color) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->color = color; \
	} \
	void SetVertexColorRGB(size_t index, uint8_t r, uint8_t g, uint8_t b) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		auto& color = vertex->color; \
		color = ColorAccess::SetColorR(color, r); \
		color = ColorAccess::SetColorG(color, g); \
		color = ColorAccess::SetColorB(color, b); \
	} \
	void SetVertexColorARGB(size_t index, uint8_t a, uint8_t r, uint8_t g, uint8_t b) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		vertex->color = COLOR_ARGB(a,r,g,b); \
	} \
	void SetVertexAlpha(size_t index, uint8_t a) \
	{ \
		auto vertex = GetVertex(index); \
		if (vertex == nullptr) \
			return; \
		auto& color = vertex->color; \
		color = ColorAccess::SetColorA(color, a); \
	} \
	void SetColorRGB(uint32_t color) \
	{ \
		auto r = ColorAccess::GetColorR(color); \
		auto g = ColorAccess::GetColorG(color); \
		auto b = ColorAccess::GetColorB(color); \
		for (auto iVert = 0; iVert < vertex_.size(); iVert++) \
		{ \
			SetVertexColorRGB(iVert, r, g, b); \
		} \
	} \
	void SetAlpha(uint8_t alpha) \
	{ \
		for (auto iVert = 0; iVert < vertex_.size(); iVert++) \
		{ \
			SetVertexAlpha(iVert, alpha); \
		} \
	}

}

#endif /* __DIRECTX_VERTEXDECL__ */
