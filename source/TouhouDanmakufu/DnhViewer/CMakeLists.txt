set(VIEWER_SOURCES Common.cpp
DebugWindow.cpp
GcLibImpl.cpp
MainWindow.cpp
ScenePanel.cpp
ScriptSelect.cpp
WinMain.cpp
Common.hpp
Constant.hpp
DebugWindow.hpp
GcLibImpl.hpp
MainWindow.hpp
ScenePanel.hpp
ScriptSelect.hpp)

add_executable(DnhViewer ${VIEWER_SOURCES})
target_link_libraries(DnhViewer PRIVATE DnhCommon)