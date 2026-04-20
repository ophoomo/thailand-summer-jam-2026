cmake_minimum_required(VERSION 4.3.1)

set(LANG_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(LANG_DST "${CMAKE_BINARY_DIR}/src/game/assets/lang")

file(GLOB_RECURSE LANG_FILES "${LANG_SRC}/*.json")

add_custom_target(copy_lang ALL
    COMMENT "Copying Lang Config to binary folder"
)

foreach(f ${LANG_FILES})
    file(RELATIVE_PATH rel "${LANG_SRC}" "${f}")
    set(dest "${LANG_DST}/${rel}")
    get_filename_component(dest_dir "${dest}" DIRECTORY)
    add_custom_command(TARGET copy_lang PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dest_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy "${f}" "${dest}"
        COMMENT "Copying ${rel}"
    )
endforeach()
