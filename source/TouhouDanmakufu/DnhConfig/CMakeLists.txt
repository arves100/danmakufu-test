set(CONFIG_SOURCES Common.cpp Common.hpp Constant.hpp GcLibImpl.cpp GcLibImpl.hpp MainWindow.cpp MainWindow.hpp WinMain.cpp)
add_executable(DnhConfig ${CONFIG_SOURCES})
target_link_libraries(DnhConfig PRIVATE DnhCommon)
