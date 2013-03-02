set(SDL2_BUILD_DIR ${CMAKE_BINARY_DIR}/lib/SDL)
set(SDL2_SOURCE_DIR ${LOCAL_LIBRARY}/SDL)
set(SDL2_INCLUDE_DIR ${SDL2_BUILD_DIR}/include ${SDL2_SOURCE_DIR}/include)
if(MSVC OR XCODE)
	set(SDL2_BUILD_DIR ${SDL2_BUILD_DIR}/${CMAKE_BUILD_TYPE})
endif()
set(SDL2_LIBRARIES ${SDL2_BUILD_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX})

if(MINGW OR UNIX)
	execute_process(COMMAND rm -f include WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
	execute_process(COMMAND ln -s lib/SDL/include WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
	list(APPEND EXTRA_CFLAGS "-w")
	if(APPLE)
		find_library(CARBON_LIBRARY Carbon REQUIRED)
		find_library(COCOA_LIBRARY Cocoa REQUIRED)
		find_library(FORCEFEEDBACK_LIBRARY ForceFeedback REQUIRED)
		find_library(IOKIT_LIBRARY IOKit REQUIRED)
		set(PLATFORM_LIBRARIES
				${CARBON_LIBRARY} ${COCOA_LIBRARY} ${FORCEFEEDBACK_LIBRARY}
				${IOKIT_LIBRARY} ${PLATFORM_LIBRARIES})
	elseif(MINGW)
		set(PLATFORM_LIBRARIES
				mingw32 ${SDL2_BUILD_DIR}/libSDL2main.a gdi32 imm32 ole32
				oleaut32 uuid version winmm ${PLATFORM_LIBRARIES})
	else()
		list(APPEND EXTRA_CFLAGS "-fPIC")
		set(PLATFORM_LIBRARIES dl ${PLATFORM_LIBRARIES})
	endif()
elseif(MSVC)
	set(PLATFORM_LIBRARIES ${SDL2_BUILD_DIR}/SDL2main.lib imm32 version winmm ${PLATFORM_LIBRARIES})
endif()

set(SDL_ATOMIC OFF CACHE BOOL "Enable the atomic operations subsystem")
set(SDL_AUDIO OFF CACHE BOOL "Enable the audio subsystem")
set(SDL_RENDER OFF CACHE BOOL "Enable the render subsystem")
set(SDL_THREADS OFF CACHE BOOL "Enable the threading subsystem")

set(CLOCK_GETTIME ON CACHE BOOL "Use clock_gettime() instead of gettimeofday()")
set(DIRECTX OFF CACHE BOOL "Use DirectX for Windows audio/video")
set(VIDEO_DUMMY OFF CACHE BOOL "Use dummy video driver")
set(VIDEO_X11_XCURSOR OFF CACHE BOOL "Enable Xcursor support")
set(VIDEO_X11_XSHAPE OFF CACHE BOOL "Enable Xshape support")

set(SDL_SHARED OFF CACHE BOOL "Build a shared version of the library")

add_subdirectory(${SDL2_SOURCE_DIR})
add_definitions(-DUSING_GENERATED_CONFIG_H)
add_dependencies(rainbow SDL2-static SDL2main)