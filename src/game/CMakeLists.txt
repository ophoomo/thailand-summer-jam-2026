
# ---------------------------------------------------------------------------
# Gather source files
# ---------------------------------------------------------------------------
file(GLOB_RECURSE GAME_SOURCES
    app/*.cpp
    ecs/**/*.cpp
    scenes/*.cpp
    config/*.cpp
)

# ---------------------------------------------------------------------------
# Create the executable target
# ---------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${GAME_SOURCES})

# ---------------------------------------------------------------------------
# Configure include directories
# ---------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# ---------------------------------------------------------------------------
# Link required libraries
# ---------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE OxEngine)
