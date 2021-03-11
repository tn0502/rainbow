#!/bin/bash
# Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
# Distributed under the MIT License.
# (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

project_root=$(cd -P "$(dirname $0)/.." && pwd)

if [[ $(git ls-files --error-unmatch . 2> /dev/null) ]] && \
   [[ "$(git rev-parse --show-toplevel 2> /dev/null)" == "$project_root" ]]; then
  echo "$0: Cannot run while still inside the repository"
  exit 0
fi

function compile {
  case "$1" in
    "Ninja")
      $ANALYZER ninja
      ;;
    "Unix Makefiles")
      $ANALYZER make
      ;;
    "Xcode")
      open Rainbow.xcodeproj
      ;;
    *)
      ;;
  esac
}

function generator {
  if [[ ! -z $GENERATOR ]]; then
    echo "$GENERATOR"
  elif brew ls --versions ninja &> /dev/null || \
       pacman -Qi ninja &> /dev/null || \
       dpkg -l ninja-build &> /dev/null; then
    echo "Ninja"
  else
    echo "Unix Makefiles"
  fi
}

function run_cmake {
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $@ -G "$(generator)" "$project_root" || return 1
  return 0
}

# Prune arguments
if [[ ! -z "$2" ]]; then
  args=$@
  args=${args#* }
fi

case $1 in
  "--help")
    echo "Syntax: $(basename $0) [analyze|clean|linux|mac|windows] [option ...]"
    echo
    echo "Options:"
    echo "  -DUNIT_TESTS=1           Enable unit tests"
    echo "  -DUSE_FMOD_STUDIO=1      Enable FMOD Studio audio engine"
    echo "  -DUSE_HEIMDALL=1         Enable Heimdall debugging facilities"
    echo "  -DUSE_PHYSICS=1          Enable physics module (Box2D)"
    echo
    echo "CMake options are passed directly to CMake so you can set variables like"
    echo "-DCMAKE_BUILD_TYPE=<type> among others."
    echo
    echo "Environment variables:"
    echo "  CC                       C compiler command"
    echo "  CXX                      C++ compiler command"
    echo "  GENERATOR                CMake: Specify a build system generator"
    ;;
  "analyze")
    ANALYZER=scan-build CC=ccc-analyzer CXX=c++-analyzer $SHELL $0 $args
    ;;
  "clean")
    rm -fr \
      *-build \
      *-subbuild \
      .ninja_* \
      CMakeCache.txt \
      CMakeFiles \
      CMakeScripts \
      CPack* \
      Debug \
      Makefile \
      MinSizeRel \
      Rainbow.* \
      RelWithDebInfo \
      Release \
      _deps \
      cmake_install.cmake \
      compile_commands.json \
      exports \
      lib \
      lib*.a \
      rainbow* \
      {build,rules}.ninja
    ;;
  "emscripten")
    EMSCRIPTEN=$(em-config EMSCRIPTEN_ROOT)
    if [[ ! -d "$EMSCRIPTEN" ]]; then
      echo "$0: Could not find Emscripten"
      exit 1
    fi
    eval run_cmake -DCMAKE_TOOLCHAIN_FILE="$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake" $args &&
    compile "$(generator)"
    ;;
  "help")
    $SHELL $0 --help
    ;;
  "linux")
    eval run_cmake $args &&
    compile "$(generator)"
    ;;
  "mac")
    eval run_cmake $args &&
    compile "$(generator)"
    ;;
  "windows")
    eval run_cmake -DCMAKE_TOOLCHAIN_FILE="$project_root/build/cmake/MinGW.cmake" $args &&
    compile "$(generator)"
    ;;
  *)  # Attempt to detect platform
    case $(uname) in
      "Darwin")
        $SHELL $0 mac $@
        ;;
      "Linux")
        $SHELL $0 linux $@
        ;;
      *)
        $SHELL $0 --help
        ;;
    esac
    ;;
esac
