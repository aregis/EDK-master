project (huestream_example_console C CXX)

set(files
		main.cpp
		LightningEffect.cpp
		LightningEffect.h
		BrightnessSawEffect.cpp
		BrightnessSawEffect.h
		SawAndLightningMenu.cpp
		SawAndLightningMenu.h
		ExplosionMenu.cpp
		ExplosionMenu.h
		SequenceMenu.cpp
		SequenceMenu.h
        App.cpp
        App.h
		RGBSineEffect.cpp
		RGBSineEffect.h
		RGBSpeedControlMenu.cpp
		RGBSpeedControlMenu.h
		Terminal.cpp
		Terminal.h
        Menu.cpp Menu.h)

add_executable (huestream_example_console ${files})
include_directories(
		..
)
target_link_libraries(huestream_example_console huestream)


