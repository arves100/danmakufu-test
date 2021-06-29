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
Task.cpp
Task.hpp
Thread.cpp
Thread.hpp
Window.cpp
Window.hpp)

add_library(GcLibStd STATIC ${GCLIBSTD_SOURCES})
target_link_libraries(GcLibStd PUBLIC ${ZLIB_LIBRARIES} ${VORBIS_LIBRARIES} ${SDL2_LIBRARIES})
target_include_directories(GcLibStd PUBLIC ${SDL2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${VORBIS_INCLUDE_DIRS})

if (WIN32)
	target_link_libraries(GcLibStd PRIVATE shlwapi comctl32 winmm)
endif()
