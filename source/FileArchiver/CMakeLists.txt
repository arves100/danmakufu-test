set(FA_SOURCES Constant.hpp LibImpl.cpp LibImpl.hpp MainWindow.cpp MainWindow.hpp WinMain.cpp)
add_executable(FileArchiver ${FA_SOURCES})
target_link_libraries(FileArchiver PRIVATE GcLibStd)
