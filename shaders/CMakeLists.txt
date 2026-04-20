cmake_minimum_required(VERSION 4.3.1)

find_program(SLANGC_EXECUTABLE slangc)
if(NOT SLANGC_EXECUTABLE)
    message(FATAL_ERROR "slangc not found. Please install Slang or add it to PATH.")
endif()

set(SPIRV_DIR "${CMAKE_BINARY_DIR}/src/game/assets/shaders/")
file(MAKE_DIRECTORY ${SPIRV_DIR})

file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/shaders/*.slang")

set(SPIRV_FILES)

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_FULL_NAME ${SHADER} NAME)

    if(SHADER_FULL_NAME MATCHES "\\.vert\\.slang$")
        set(STAGE "vertex")
    elseif(SHADER_FULL_NAME MATCHES "\\.frag\\.slang$")
        set(STAGE "fragment")
    else()
        message(WARNING "Unknown shader stage for ${SHADER}, skipping")
        continue()
    endif()

    string(REGEX REPLACE "\\.slang$" ".spv" SPIRV_OUT_NAME "${SHADER_FULL_NAME}")
    set(SPIRV_OUT "${SPIRV_DIR}/${SPIRV_OUT_NAME}")

    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER} -o ${SPIRV_OUT} -entry main -stage ${STAGE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_FULL_NAME} → ${SPIRV_OUT_NAME}"
        VERBATIM
    )

    list(APPEND SPIRV_FILES ${SPIRV_OUT})
endforeach()

add_custom_target(Shaders ALL
    DEPENDS ${SPIRV_FILES}
)
