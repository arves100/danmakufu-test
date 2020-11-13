set(GCLIBSTD_SOURCES Application.cpp
Application.hpp
File.cpp
File.hpp
FpsController.cpp
FpsController.hpp
GstdConstant.hpp
GstdLib.hpp
GstdUtility.cpp
GstdUtility.hpp
Logger.cpp
Logger.hpp
MersenneTwister.cpp
MersenneTwister.hpp
Script.cpp
Script.hpp
ScriptClient.cpp
ScriptClient.hpp
SmartPointer.hpp
Task.cpp
Task.hpp
Thread.cpp
Thread.hpp
Window.cpp
Window.hpp)

add_library(GcLibStd STATIC ${GCLIBSTD_SOURCES})
target_link_libraries(GcLibStd PUBLIC ZLIB::ZLIB Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile)
target_include_directories(GcLibStd PUBLIC ${ZLIB_INCLUDE_DIRS} ${VORBIS_INCLUDE_DIR})
