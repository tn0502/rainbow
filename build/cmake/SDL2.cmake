set(SDL2_TARGET_VERSION 2.0.12)
set(SDL2_DEV_HASH 00c55a597cebdb9a4eb2723f2ad2387a4d7fd605e222c69b46099b15d5d8b32d)
set(SDL2_BIN_HASH 9d3cd8b83e62ef209b603819c138d3b7c32c860cc1eb8342e853e04c3c81dbc2)

if(ANDROID)
  # Pass through.
elseif(EMSCRIPTEN)
  set(SDL2_INCLUDE_DIRS ${LOCAL_LIBRARY}/SDL/include)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DEMANGLE_SUPPORT=1 -s USE_SDL=2")
elseif(MSVC)
  download_library(
    sdl2-bin
    https://libsdl.org/release/SDL2-${SDL2_TARGET_VERSION}-win32-x64.zip
    ${SDL2_BIN_HASH}
  )
  copy_to_build_dir(sdl2-bin SDL2.dll SDL2.dll)

  download_library(
    sdl2-dev
    https://libsdl.org/release/SDL2-devel-${SDL2_TARGET_VERSION}-VC.zip
    ${SDL2_DEV_HASH}
  )
  ExternalProject_Get_Property(sdl2-dev SOURCE_DIR)
  add_dependencies(sdl2-dev sdl2-bin)
  add_dependencies(rainbow sdl2-dev)

  target_include_directories(rainbow PRIVATE ${SOURCE_DIR}/include)
  set(SDL2_LIBRARY_DIR ${SOURCE_DIR}/lib/x64)
  target_link_libraries(rainbow
    ${SDL2_LIBRARY_DIR}/SDL2.lib
    ${SDL2_LIBRARY_DIR}/SDL2main.lib
  )
elseif(UNIX)
  pkg_check_modules(SDL2 REQUIRED sdl2)
  target_include_directories(rainbow PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(rainbow ${SDL2_LDFLAGS})
endif()
