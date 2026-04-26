
# ---------------------------------------------------------------------------
# Gather source files
# ---------------------------------------------------------------------------
file(GLOB_RECURSE GAME_SOURCES
    app/*.cpp
    ecs/**/*.cpp
    scenes/*.cpp
    config/*.cpp
    battle/*.cpp
    cards/*.cpp
    characters/*.cpp
    particles/*.cpp
    gui/*.cpp
)

# ---------------------------------------------------------------------------
# Create the executable target
# ---------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${GAME_SOURCES})

# ---------------------------------------------------------------------------
# Add icon game
# ---------------------------------------------------------------------------

if(WIN32)
    set(ICON_PATH "${CMAKE_SOURCE_DIR}/assets/images/app.ico")
    string(REPLACE "\\" "/" ICON_PATH_NATIVE "${ICON_PATH}")
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/app.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/app.rc"
            @ONLY
    )
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
endif()


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
target_link_libraries(${PROJECT_NAME} PRIVATE OxEngine SDL3::SDL3-static)
