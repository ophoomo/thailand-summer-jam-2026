cmake_minimum_required(VERSION 4.3.1)

find_program(MSDF_ATLAS_GEN_EXECUTABLE msdf-atlas-gen)

if(NOT MSDF_ATLAS_GEN_EXECUTABLE)
    message(FATAL_ERROR "msdf-atlas-gen not found. Please install it or add it to PATH.")
endif()

set(FONT_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(FONT_DST "${CMAKE_BINARY_DIR}/src/game/assets/font")

file(GLOB_RECURSE FONT_FILES "${FONT_SRC}/*.ttf")

file(MAKE_DIRECTORY ${FONT_DST})

foreach(FONT_FILE ${FONT_FILES})
    get_filename_component(FONT_NAME ${FONT_FILE} NAME_WE)

    set(OUTPUT_PNG  "${FONT_DST}/${FONT_NAME}.png")
    set(OUTPUT_JSON "${FONT_DST}/${FONT_NAME}.json")

    add_custom_command(
        OUTPUT ${OUTPUT_PNG} ${OUTPUT_JSON}
        COMMAND ${MSDF_ATLAS_GEN_EXECUTABLE}
            -font     ${FONT_FILE}
            -charset  ${CMAKE_CURRENT_SOURCE_DIR}/charset.txt
            -type     msdf
            -format   png
            -size     48
            -pxrange  4
            -square
            -yorigin  top
            -imageout ${OUTPUT_PNG}
            -json     ${OUTPUT_JSON}
        DEPENDS ${FONT_FILE}
        COMMENT "Generating MSDF atlas for ${FONT_NAME}"
    )

    add_custom_target(
        generate_${FONT_NAME}
        DEPENDS ${OUTPUT_PNG} ${OUTPUT_JSON}
    )

    list(APPEND GENERATED_FONTS generate_${FONT_NAME})
endforeach()

add_custom_target(generate_fonts ALL DEPENDS ${GENERATED_FONTS})
