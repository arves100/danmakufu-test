set (TESTS
	BgfxBump.cpp
	BgfxBump.hpp

	SpriteTest.cpp
	SpriteTest.hpp
)

make_shader_vf(06bump testmode)

add_executable(TestMode StdAfx.h TestApp.cpp TestApp.hpp ${TESTS})
target_link_libraries(TestMode PRIVATE GcLibDx GcLibStd SDL2::SDL2main)

