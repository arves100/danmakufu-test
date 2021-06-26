#pragma once

class CBgfxBump : public CTestApp
{
public:
	CBgfxBump();
	~CBgfxBump();

protected:
	bool OnInit() override;
	bool OnPreInit() override;
	void OnDestroy() override;
	void OnLoop(const float) override;
	void OnRender(const float) override;
	const char* GetTestName() const override { return "BGFX 06 Bump example"; }
private:
	int16_t m_lights;

	directx::RenderObjectLTA m_obj;
};
