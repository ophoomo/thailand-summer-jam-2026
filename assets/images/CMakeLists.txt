cmake_minimum_required(VERSION 4.3.1)

set(IMAGE_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(IMAGE_DST "${CMAKE_BINARY_DIR}/src/game/assets/images")

file(GLOB_RECURSE IMAGE_FILES "${IMAGE_SRC}/*.png")

add_custom_target(copy_image ALL
    COMMENT "Copying Image to binary folder"
)

foreach(f ${IMAGE_FILES})
    file(RELATIVE_PATH rel "${IMAGE_SRC}" "${f}")
    set(dest "${IMAGE_DST}/${rel}")
    get_filename_component(dest_dir "${dest}" DIRECTORY)
    add_custom_command(TARGET copy_image PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dest_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy "${f}" "${dest}"
        COMMENT "Copying ${rel}"
    )
endforeach()
