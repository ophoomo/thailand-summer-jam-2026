
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
# Hide terminal window in Release build (Windows only)
# ---------------------------------------------------------------------------
if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,RelWithDebInfo>:/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup>
    )
endif()

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
