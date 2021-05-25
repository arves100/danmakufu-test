set(EXECUTOR_SOURCES Common.hpp
Constant.hpp
GcLibImpl.hpp
ScriptSelectScene.hpp
StgScene.hpp
System.hpp
TitleScene.hpp
Common.cpp
GcLibImpl.cpp
ScriptSelectScene.cpp
StgScene.cpp
System.cpp
TitleScene.cpp
WinMain.cpp)

if (WIN32)
	list(APPEND EXECUTOR_SOURCES DnhExecuter.rc resource.h)
endif()

add_executable(DnhExecutor ${EXECUTOR_SOURCES})
target_link_libraries(DnhExecutor PRIVATE DnhCommon ${SDL2_LIBRARIES})
target_include_directories(DnhExecutor PRIVATE ${CMAKE_BINARY_DIR})
