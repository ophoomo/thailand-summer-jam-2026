cmake_minimum_required(VERSION 4.3.1)
project(heliora)

# ---------------------------------------------------------------------------
# C++ Standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# OpenAL-soft → force static build
# ---------------------------------------------------------------------------
set(LIBTYPE "STATIC" CACHE STRING "" FORCE)
set(ALSOFT_UTILS      OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS      OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL    OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)

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
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(IS_MULTI_CONFIG)
    message(STATUS "Multi-config generator detected — assets/shaders added for all configs")
    add_subdirectory(shaders)
    add_subdirectory(assets)
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Debug build — adding shaders and assets")
        add_subdirectory(shaders)
        add_subdirectory(assets)
    endif()
endif()

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
add_subdirectory(src/engine)
add_subdirectory(src/game)
