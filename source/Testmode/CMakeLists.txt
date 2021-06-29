set (TESTS
	BgfxBump.cpp
	BgfxBump.hpp

	SpriteTest.cpp
	SpriteTest.hpp
)

make_shader_vf(06bump testmode)

add_executable(TestMode StdAfx.h TestApp.cpp TestApp.hpp ${TESTS})

if (DEFINED VCPKG_TARGET_TRIPLET)
	set(SDL2MAIN_LIB SDL2::SDL2main)
else()
	set(SDL2MAIN_LIB SDL2main)
endif()

target_link_libraries(TestMode PRIVATE GcLibDx GcLibStd ${SDL2MAIN_LIB})

