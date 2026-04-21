cmake_minimum_required(VERSION 4.3.1)
project(oxen_assets)

# ---------------------------------------------------------------------------
# External Library Configuration
# ---------------------------------------------------------------------------
set(OXEN_ASSETS_NAME oxen_assets)
set(OXEN_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# ---------------------------------------------------------------------------
# Select Library File Based on Platform
# ---------------------------------------------------------------------------
if(WIN32)
    set(OXEN_ASSETS_FILE ${OXEN_ASSETS_PATH}/${OXEN_ASSETS_NAME}.lib)
elseif(APPLE)
    set(OXEN_ASSETS_FILE ${OXEN_ASSETS_PATH}/lib${OXEN_ASSETS_NAME}.dylib)
else()
    set(OXEN_ASSETS_FILE ${OXEN_ASSETS_PATH}/lib${OXEN_ASSETS_NAME}.so)
endif()

# ---------------------------------------------------------------------------
# Configure include directories
# ---------------------------------------------------------------------------
target_include_directories(${OXEN_ASSETS_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# Link External Library
# ---------------------------------------------------------------------------
target_link_libraries(${OXEN_ASSETS_NAME} PUBLIC ${OXEN_ASSETS_FILE})
