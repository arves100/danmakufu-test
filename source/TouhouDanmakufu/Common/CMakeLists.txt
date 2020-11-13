set(COMMON_SOURCES DnhCommon.cpp DnhCommon.hpp DnhConstant.hpp DnhGcLibImpl.hpp DnhGcLibImpl.cpp
	DnhReplay.cpp DnhReplay.hpp DnhScript.cpp DnhScript.hpp StgCommon.cpp StgCommon.hpp
	StgControlScript.cpp StgControlScript.hpp StgEnemy.cpp StgEnemy.hpp StgIntersection.cpp StgIntersection.hpp
	StgItem.cpp StgItem.hpp StgPackageController.cpp StgPackageController.hpp StgPackageScript.cpp StgPackageScript.hpp
	StgPlayer.cpp StgPlayer.hpp StgShot.cpp StgShot.hpp StgStageController.cpp StgStageController.hpp StgStageScript.cpp
	StgStageScript.hpp StgSystem.cpp StgSystem.hpp StgUserExtendScene.cpp StgUserExtendScene.hpp)

add_library(DnhCommon STATIC ${COMMON_SOURCES})
target_link_libraries(DnhCommon PUBLIC GcLibDx)
