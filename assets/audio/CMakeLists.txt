cmake_minimum_required(VERSION 4.3.1)

set(AUDIO_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(AUDIO_DST "${CMAKE_BINARY_DIR}/src/game/assets/audio")

file(GLOB_RECURSE AUDIO_FILES "${AUDIO_SRC}/*.ogg")

add_custom_target(copy_audio ALL
    COMMENT "Copying Audio to binary folder"
)

foreach(f ${AUDIO_FILES})
    file(RELATIVE_PATH rel "${AUDIO_SRC}" "${f}")
    set(dest "${AUDIO_DST}/${rel}")
    get_filename_component(dest_dir "${dest}" DIRECTORY)
    add_custom_command(TARGET copy_audio PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dest_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy "${f}" "${dest}"
        COMMENT "Copying ${rel}"
    )
endforeach()
