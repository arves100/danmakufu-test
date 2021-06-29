#ifndef __DIRECTX_UTILITY__
#define __DIRECTX_UTILITY__

#include "DxConstant.hpp"
#include <bx/pixelformat.h>

namespace directx {

/**********************************************************
//ColorAccess
**********************************************************/
class ColorAccess {
public:
	static uint8_t GetColorA(uint32_t& color);
	static uint32_t& SetColorA(uint32_t& color, uint8_t alpha);
	static uint8_t GetColorR(uint32_t color);
	static uint32_t& SetColorR(uint32_t& color, uint8_t red);
	static uint8_t GetColorG(uint32_t& color);
	static uint32_t& SetColorG(uint32_t& color, uint8_t green);
	static uint8_t GetColorB(uint32_t& color);
	static uint32_t& SetColorB(uint32_t& color, uint8_t blue);

	static uint32_t& ApplyAlpha(uint32_t& color, double alpha);

};

#if 0
/**********************************************************
//衝突判定用図形
**********************************************************/
class DxCircle {
public:
	DxCircle()
	{
		x_ = 0;
		y_ = 0;
		r_ = 0;
	}
	DxCircle(double x, double y, double r)
	{
		x_ = x;
		y_ = y;
		r_ = r;
	}
	virtual ~DxCircle() {}
	double GetX() { return x_; }
	void SetX(float x) { x_ = x; }
	double GetY() { return y_; }
	void SetY(float y) { y_ = y; }
	double GetR() { return r_; }
	void SetR(float r) { r_ = r; }

private:
	double x_;
	double y_;
	double r_;
};

class DxWidthLine {
	//幅のある線分
public:
	DxWidthLine()
	{
		posX1_ = 0;
		posY1_ = 0;
		posX2_ = 0;
		posY2_ = 0;
		width_ = 0;
	}
	DxWidthLine(double x1, double y1, double x2, double y2, double width)
	{
		posX1_ = x1;
		posY1_ = y1;
		posX2_ = x2;
		posY2_ = y2;
		width_ = width;
	}
	virtual ~DxWidthLine() {}
	double GetX1() { return posX1_; }
	double GetY1() { return posY1_; }
	double GetX2() { return posX2_; }
	double GetY2() { return posY2_; }
	double GetWidth() { return width_; }

private:
	double posX1_;
	double posY1_;
	double posX2_;
	double posY2_;
	double width_;
};

class DxLine3D {
public:
	DxLine3D(){};
	DxLine3D(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2)
	{
		vertex_[0] = p1;
		vertex_[1] = p2;
	}

	D3DXVECTOR3& GetPosition(int index) { return vertex_[index]; }
	D3DXVECTOR3& GetPosition1() { return vertex_[0]; }
	D3DXVECTOR3& GetPosition2() { return vertex_[1]; }

private:
	D3DXVECTOR3 vertex_[2];
};

class DxTriangle {
public:
	DxTriangle() {}
	DxTriangle(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, const D3DXVECTOR3& p3)
	{
		vertex_[0] = p1;
		vertex_[1] = p2;
		vertex_[2] = p3;
		_Compute();
	}

	D3DXVECTOR3& GetPosition(int index) { return vertex_[index]; }
	D3DXVECTOR3& GetPosition1() { return vertex_[0]; }
	D3DXVECTOR3& GetPosition2() { return vertex_[1]; }
	D3DXVECTOR3& GetPosition3() { return vertex_[2]; }

private:
	D3DXVECTOR3 vertex_[3];
	D3DXVECTOR3 normal_;

	void _Compute()
	{
		D3DXVECTOR3 lv[3];
		D3DXVECTOR3 dm;
		lv[0] = vertex_[1] - vertex_[0];
		lv[0] = *D3DXVec3Normalize(&dm, &lv[0]);

		lv[1] = vertex_[2] - vertex_[1];
		lv[1] = *D3DXVec3Normalize(&dm, &lv[1]);

		lv[2] = vertex_[0] - vertex_[2];
		lv[2] = *D3DXVec3Normalize(&dm, &lv[2]);

		D3DXVECTOR3 cross = *D3DXVec3Cross(&dm, &lv[0], &lv[1]);
		normal_ = *D3DXVec3Normalize(&dm, &cross);
	}
};

/**********************************************************
//DxMath
**********************************************************/
class DxMath {
public:
	//衝突判定：点－多角形
	static bool IsIntersected(D3DXVECTOR2& pos, std::vector<D3DXVECTOR2>& list);

	//衝突判定：円-円
	static bool IsIntersected(DxCircle& circle1, DxCircle& circle2);

	//衝突判定：円-直線
	static bool IsIntersected(DxCircle& circle, DxWidthLine& line);

	//衝突判定：直線-直線
	static bool IsIntersected(DxWidthLine& line1, DxWidthLine& line2);

	//衝突判定：直線：三角
	static bool IsIntersected(DxLine3D& line, std::vector<DxTriangle>& triangles, std::vector<D3DXVECTOR3>& out);
};
#endif

struct RECT_F
{
	float left;
	float top;
	float right;
	float bottom;

	RECT_F() : left(0.0f), top(0.0f), right(0.0f), bottom(0.0f) {}
	RECT_F(float left, float top, float right, float bottom) : left(left), top(top), right(right), bottom(bottom) {}
};

class DxAllocator
{
public:
	DxAllocator() : alloc(new bx::DefaultAllocator())
	{
		instance_ = this;
	}

	virtual ~DxAllocator()
	{
		if (alloc)
		{
			delete alloc;
			alloc = nullptr;
		}

		instance_ = nullptr;
	}

	static bx::AllocatorI* Get()
	{
		return instance_->alloc;
	}

private:
	bx::AllocatorI* alloc;
	static DxAllocator* instance_;
};

#define COLOR_RGB(r,g,b) (uint32_t)((r << 0x18) | (g << 0x10) | (b << 8) | 0xFF)
#define COLOR_ARGB(a,r,g,b) (uint32_t)((r << 0x18) | (g << 0x10) | (b << 8) | a)

// https://github.com/bkaradzic/bgfx/blob/d01f86a6a9cac2e7c97c13983887f0ca4994318a/examples/common/bgfx_utils.h
inline uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f)
{
	const float src[] =
	{
		_x * 0.5f + 0.5f,
		_y * 0.5f + 0.5f,
		_z * 0.5f + 0.5f,
		_w * 0.5f + 0.5f,
	};
	uint32_t dst;
	bx::packRgba8(&dst, src);
	return dst;
}

static void calcTangents(void* _vertices, uint16_t _numVertices, bgfx::VertexLayout _layout, const uint16_t* _indices, uint32_t _numIndices)
{
	struct PosTexcoord
	{
		float m_x;
		float m_y;
		float m_z;
		float m_pad0;
		float m_u;
		float m_v;
		float m_pad1;
		float m_pad2;
	};

	float* tangents = new float[6*_numVertices];
	bx::memSet(tangents, 0, 6*_numVertices*sizeof(float) );

	PosTexcoord v0;
	PosTexcoord v1;
	PosTexcoord v2;

	for (uint32_t ii = 0, num = _numIndices/3; ii < num; ++ii)
	{
		const uint16_t* indices = &_indices[ii*3];
		uint32_t i0 = indices[0];
		uint32_t i1 = indices[1];
		uint32_t i2 = indices[2];

		bgfx::vertexUnpack(&v0.m_x, bgfx::Attrib::Position, _layout, _vertices, i0);
		bgfx::vertexUnpack(&v0.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i0);

		bgfx::vertexUnpack(&v1.m_x, bgfx::Attrib::Position, _layout, _vertices, i1);
		bgfx::vertexUnpack(&v1.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i1);

		bgfx::vertexUnpack(&v2.m_x, bgfx::Attrib::Position, _layout, _vertices, i2);
		bgfx::vertexUnpack(&v2.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i2);

		const float bax = v1.m_x - v0.m_x;
		const float bay = v1.m_y - v0.m_y;
		const float baz = v1.m_z - v0.m_z;
		const float bau = v1.m_u - v0.m_u;
		const float bav = v1.m_v - v0.m_v;

		const float cax = v2.m_x - v0.m_x;
		const float cay = v2.m_y - v0.m_y;
		const float caz = v2.m_z - v0.m_z;
		const float cau = v2.m_u - v0.m_u;
		const float cav = v2.m_v - v0.m_v;

		const float det = (bau * cav - bav * cau);
		const float invDet = 1.0f / det;

		const float tx = (bax * cav - cax * bav) * invDet;
		const float ty = (bay * cav - cay * bav) * invDet;
		const float tz = (baz * cav - caz * bav) * invDet;

		const float bx = (cax * bau - bax * cau) * invDet;
		const float by = (cay * bau - bay * cau) * invDet;
		const float bz = (caz * bau - baz * cau) * invDet;

		for (uint32_t jj = 0; jj < 3; ++jj)
		{
			float* tanu = &tangents[indices[jj]*6];
			float* tanv = &tanu[3];
			tanu[0] += tx;
			tanu[1] += ty;
			tanu[2] += tz;

			tanv[0] += bx;
			tanv[1] += by;
			tanv[2] += bz;
		}
	}

	for (uint32_t ii = 0; ii < _numVertices; ++ii)
	{
		const bx::Vec3 tanu = bx::load<bx::Vec3>(&tangents[ii*6]);
		const bx::Vec3 tanv = bx::load<bx::Vec3>(&tangents[ii*6 + 3]);

		float nxyzw[4];
		bgfx::vertexUnpack(nxyzw, bgfx::Attrib::Normal, _layout, _vertices, ii);

		const bx::Vec3 normal  = bx::load<bx::Vec3>(nxyzw);
		const float    ndt     = bx::dot(normal, tanu);
		const bx::Vec3 nxt     = bx::cross(normal, tanu);
		const bx::Vec3 tmp     = bx::sub(tanu, bx::mul(normal, ndt) );

		float tangent[4];
		bx::store(tangent, bx::normalize(tmp) );
		tangent[3] = bx::dot(nxt, tanv) < 0.0f ? -1.0f : 1.0f;

		bgfx::vertexPack(tangent, true, bgfx::Attrib::Tangent, _layout, _vertices, ii);
	}

	delete [] tangents;
}

} // namespace directx

#endif
