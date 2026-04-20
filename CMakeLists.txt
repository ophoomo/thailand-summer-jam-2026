cmake_minimum_required(VERSION 4.3.1)
project(tsj2026)

# ---------------------------------------------------------------------------
# C++ Standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Find libraries
# ---------------------------------------------------------------------------
find_package(Vulkan REQUIRED)

# ---------------------------------------------------------------------------
# External libraries (submodules)
# ---------------------------------------------------------------------------
add_subdirectory(external/spdlog)
add_subdirectory(external/entt)
add_subdirectory(external/json)
add_subdirectory(external/SDL3)
add_subdirectory(external/openal)
add_subdirectory(external/box2d)
add_subdirectory(external/lua)
add_subdirectory(external/glm)
add_subdirectory(external/imgui)
add_subdirectory(external/stb)
#add_subdirectory(external/oxen_assets)


# ---------------------------------------------------------------------------
# Debug Mode
# ---------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
message(STATUS "Adding shaders and scripts for Debug build")
add_subdirectory(shaders)
add_subdirectory(assets)
endif()

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
add_subdirectory(src/engine)
add_subdirectory(src/game)
