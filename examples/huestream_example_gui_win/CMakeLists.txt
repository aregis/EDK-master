#Streamtest app uses MFC, only for Windows with Visual Studio
if (WIN32 AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    ADD_DEFINITIONS(-D_AFXDLL)
    ADD_DEFINITIONS(-DNO_WARN_MBCS_MFC_DEPRECATION)
    SET(CMAKE_MFC_FLAG 2)

	include_directories(
		..
)
    set(prjname "huestream_example_gui_win")
    project (${prjname} C CXX)
    add_executable (${prjname} WIN32 
    "StreamTest.cpp" "StreamTest.h" 
    "StreamTestDlg.cpp" "StreamTestDlg.h" 
    "stdafx.cpp" "stdafx.h" 
    "StreamTest.rc" "res/StreamTest.ico" "res/StreamTest.rc2" "resource.h"
    "EffectPlayer.cpp" "EffectPlayer.h"
    "BrightnessSawEffect.cpp" "BrightnessSawEffect.h"
    "LightpointMarqueeEffect.cpp" "LightpointMarqueeEffect.h"
    "RGBSineEffect.cpp" "RGBSineEffect.h"
    "BrightnessSquareWaveEffect.cpp" "BrightnessSquareWaveEffect.h"
    "ColorloopEffect.cpp" "ColorloopEffect.h"
    "FireballsEffect.cpp" "FireballsEffect.h")
    
    target_link_libraries(${prjname} huestream)
    copy_libs_to_output(${prjname} "${shared_libs}")
    
    add_custom_command(TARGET ${prjname} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy ${PROJECT_SOURCE_DIR}/res/explosion.wav ${BUILD_DIR})    

endif()