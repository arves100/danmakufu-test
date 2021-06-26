#pragma once

class CSpriteTest : public CTestApp
{
public:
	CSpriteTest();
	~CSpriteTest();

protected:
	bool OnInit() override;
	bool OnPreInit() override;
	void OnRender() override;
	void OnDestroy() override;
	void OnLoop(const float) override;
	void OnRender(const float) override;

	directx::Sprite2D* s2;
};
