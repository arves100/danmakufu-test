#pragma once

class CSpriteTest : public CTestApp
{
public:
	CSpriteTest();
	~CSpriteTest();

protected:
	bool OnInit() override;
	bool OnPreInit() override;
	void OnDestroy() override;
	void OnLoop(const float) override;
	void OnRender(const float) override;
	const char* GetTestName() const override { return "Sprite Test"; }

	directx::Sprite2D* s2;
};
