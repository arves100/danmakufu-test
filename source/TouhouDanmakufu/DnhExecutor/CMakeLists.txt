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

add_executable(DnhExecutor ${EXECUTOR_SOURCES})
target_link_libraries(DnhExecutor PRIVATE DnhCommon)