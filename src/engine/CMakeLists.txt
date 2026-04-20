
# ---------------------------------------------------------------------------
# Gather source files
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES
    audio/**/*.cpp
    assets/*.cpp
    assets/**/*.cpp
    core/*.cpp
    renderer/*.cpp
    renderer/**/*.cpp
    scripting/*.cpp
    ui/*.cpp
    ui/**/*.cpp
    utils/*.cpp
    particles/*.cpp
    animation/*.cpp
)

# ---------------------------------------------------------------------------
# Create a static library
# ---------------------------------------------------------------------------
add_library(OxEngine STATIC ${ENGINE_SOURCES})

# ---------------------------------------------------------------------------
# Configure include directories
# ---------------------------------------------------------------------------
target_include_directories(OxEngine PUBLIC
 ${CMAKE_SOURCE_DIR}/include/engine
)

# ---------------------------------------------------------------------------
# Link required libraries
# ---------------------------------------------------------------------------
target_link_libraries(OxEngine PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    SDL3::SDL3
    OpenAL
    box2d::box2d
    lua
    glm::glm
    imgui
    EnTT::EnTT
    Vulkan::Vulkan
    stb
)
