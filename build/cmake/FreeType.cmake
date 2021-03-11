if(EMSCRIPTEN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_FREETYPE=1 -s USE_HARFBUZZ=1")
else()
  # Build FreeType without HarfBuzz to break the circular dependency
  set(FREETYPE_INCLUDE_DIR_freetype2 ${LOCAL_LIBRARY}/FreeType/include)
  set(FREETYPE_INCLUDE_DIR_ft2build ${THIRD_PARTY}/FreeType)
  add_library(freetype-bootstrap STATIC ${THIRD_PARTY}/FreeType/freetype.c)
  target_compile_definitions(freetype-bootstrap
    PRIVATE
      RAINBOW_BUILD_FREETYPE_WITHOUT_HARFBUZZ
  )
  target_include_directories(freetype-bootstrap
    PUBLIC
      ${FREETYPE_INCLUDE_DIR_ft2build}
      ${FREETYPE_INCLUDE_DIR_freetype2}
    PRIVATE
      ${LOCAL_LIBRARY}/FreeType/builds
      ${LOCAL_LIBRARY}/FreeType/src
  )

  # Build HarfBuzz with FreeType
  set(FREETYPE_LIBRARY freetype-bootstrap)
  set(HB_HAVE_FREETYPE ON CACHE INTERNAL "Enable freetype interop helpers")
  set(HB_BUILD_SUBSET OFF CACHE INTERNAL "Build harfbuzz-subset")
  set(HB_BUILD_TESTS OFF CACHE INTERNAL "Build harfbuzz tests")
  set(SKIP_INSTALL_ALL ON)
  add_subdirectory(${LOCAL_LIBRARY}/harfbuzz EXCLUDE_FROM_ALL)

  # Finally, build FreeType _with_ HarfBuzz
  add_library(freetype STATIC ${THIRD_PARTY}/FreeType/freetype.c)
  target_include_directories(freetype
    PUBLIC
      ${THIRD_PARTY}/FreeType
      ${LOCAL_LIBRARY}/FreeType/include
      ${LOCAL_LIBRARY}/harfbuzz/src
    PRIVATE
      ${LOCAL_LIBRARY}/FreeType/builds
      ${LOCAL_LIBRARY}/FreeType/src
  )

  add_dependencies(harfbuzz freetype-bootstrap)
  target_link_libraries(freetype harfbuzz)
  target_link_libraries(rainbow freetype)

  if(APPLE)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    find_library(CORETEXT_FRAMEWORK CoreText REQUIRED)
    target_link_libraries(rainbow ${CORETEXT_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK})
  endif()
endif()
