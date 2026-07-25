cmake_minimum_required(VERSION 3.20)
project(kobo_2048 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build flavors:
#   -DKOBO_2048_BACKEND=none   host: core + tests only
#   -DKOBO_2048_BACKEND=sdl    host: desktop simulator
#   -DKOBO_2048_BACKEND=fbink  device: Kobo e-ink build (cross, cmake/kobo-toolchain.cmake)
set(KOBO_2048_BACKEND "none" CACHE STRING "Rendering backend: none | sdl | fbink")
option(BUILD_TESTS "Build host unit tests" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# --- Portable core: pure logic, no OS calls ---
add_library(kobo_2048_core STATIC
  src/core/board.cpp
  src/core/game_session.cpp
  src/core/best_score.cpp
  src/core/display_settings.cpp
)
target_include_directories(kobo_2048_core PUBLIC src third_party)

# --- Persistence: atomic JSON files ---
add_library(kobo_2048_persist STATIC
  src/persist/paths.cpp
  src/persist/store.cpp
)
target_link_libraries(kobo_2048_persist PUBLIC kobo_2048_core)

# --- UI: renderer-agnostic screens and widgets ---
add_library(kobo_2048_ui STATIC
  src/ui/theme.cpp
  src/ui/widgets.cpp
  src/ui/gesture.cpp
  src/ui/tile_theme.cpp
  src/ui/screens/game_screen.cpp
  src/ui/screens/settings_screen.cpp
)
target_link_libraries(kobo_2048_ui PUBLIC kobo_2048_core)

if(KOBO_2048_BACKEND STREQUAL "sdl")
  find_package(SDL2 CONFIG QUIET)
  if(NOT SDL2_FOUND AND MSVC)
    # Prebuilt VC development package (host simulator only).
    include(FetchContent)
    FetchContent_Declare(sdl2_prebuilt
      URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.11/SDL2-devel-2.30.11-VC.zip
    )
    FetchContent_MakeAvailable(sdl2_prebuilt)
    set(SDL2_DIR "${sdl2_prebuilt_SOURCE_DIR}/cmake")
    find_package(SDL2 CONFIG REQUIRED)
  elseif(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
  endif()

  add_executable(kobo_2048
    src/main.cpp
    src/platform/softcanvas.cpp
    src/platform/sdl/sdl_renderer.cpp
    src/platform/sdl/mouse_touch.cpp
  )
  target_compile_definitions(kobo_2048 PRIVATE KOBO_2048_BACKEND_SDL SDL_MAIN_HANDLED)
  target_link_libraries(kobo_2048 PRIVATE kobo_2048_ui kobo_2048_persist SDL2::SDL2)
  if(TARGET SDL2::SDL2 AND MSVC)
    add_custom_command(TARGET kobo_2048 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:kobo_2048>)
  endif()
elseif(KOBO_2048_BACKEND STREQUAL "fbink")
  # Cross build (cmake/kobo-toolchain.cmake). FBInk is built by
  # tools/build-fbink.sh into third_party/FBInk/Release/.
  set(FBINK_LIB "${CMAKE_SOURCE_DIR}/third_party/FBInk/Release/libfbink.a")
  if(NOT EXISTS "${FBINK_LIB}")
    message(FATAL_ERROR "FBInk not built. Run tools/build-fbink.sh first (see docs/building.md).")
  endif()

  # FBInk's KOBO build resolves I2C (e.g. query_accelerometer) against its own
  # vendored/staged libi2c.a rather than bundling those symbols into
  # libfbink.a itself, so consumers must link it too.
  set(FBINK_I2C_LIB "${CMAKE_SOURCE_DIR}/third_party/FBInk/libi2c-staged/lib/libi2c.a")
  if(NOT EXISTS "${FBINK_I2C_LIB}")
    message(FATAL_ERROR "FBInk's staged libi2c.a not found. Run tools/build-fbink.sh first (see docs/building.md).")
  endif()

  add_executable(kobo_2048
    src/main.cpp
    src/platform/softcanvas.cpp
    src/platform/kobo/fbink_renderer.cpp
    src/platform/kobo/evdev_touch.cpp
  )
  target_compile_definitions(kobo_2048 PRIVATE KOBO_2048_BACKEND_FBINK)
  target_include_directories(kobo_2048 PRIVATE third_party/FBInk)
  target_link_libraries(kobo_2048 PRIVATE kobo_2048_ui kobo_2048_persist "${FBINK_LIB}" "${FBINK_I2C_LIB}" m rt pthread)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_executable(kobo_2048_tests
    tests/main.cpp
    tests/test_smoke.cpp
    tests/test_persist.cpp
    tests/test_board.cpp
    tests/test_gesture.cpp
    tests/test_game_session.cpp
    tests/test_best_score.cpp
    tests/test_display_settings.cpp
  )
  target_link_libraries(kobo_2048_tests PRIVATE kobo_2048_core kobo_2048_persist kobo_2048_ui)
  add_test(NAME core COMMAND kobo_2048_tests)
endif()
