cmake_minimum_required(VERSION 3.28)

project(Kori VERSION 0.10.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(kori MODULE)

target_sources(
  kori
  PRIVATE
    src/plugin-main.cpp
    src/core/animation-engine.cpp
    src/core/easing.cpp
    src/obs/main-canvas-target-resolver.cpp
    src/settings/focus-preview-widget.cpp
    src/settings/settings-dialog.cpp
    src/settings/settings-store.cpp
)

target_include_directories(kori PRIVATE src)
target_link_libraries(
  kori
  PRIVATE
    OBS::libobs
    OBS::obs-frontend-api
    Qt6::Widgets
)
target_compile_definitions(
  kori
  PRIVATE
    KORI_VERSION="${PROJECT_VERSION}"
)

if(MSVC)
  target_compile_options(kori PRIVATE /W4 /permissive- /EHsc)
endif()

set_target_properties(
  kori
  PROPERTIES
    PREFIX ""
    OUTPUT_NAME "kori"
)

install(
  TARGETS kori
  RUNTIME DESTINATION "obs-plugins/64bit"
  LIBRARY DESTINATION "obs-plugins/64bit"
)

install(
  DIRECTORY data/
  DESTINATION "data/obs-plugins/kori"
)
