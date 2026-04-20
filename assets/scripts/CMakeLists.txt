cmake_minimum_required(VERSION 4.3.1)

set(SCRIPT_SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(SCRIPT_DST "${CMAKE_BINARY_DIR}/src/game/assets/scripts")

file(GLOB_RECURSE LUA_FILES "${SCRIPT_SRC}/*.lua")

add_custom_target(copy_lua ALL
    COMMENT "Copying Lua scripts to binary folder"
)

foreach(f ${LUA_FILES})
    file(RELATIVE_PATH rel "${SCRIPT_SRC}" "${f}")
    set(dest "${SCRIPT_DST}/${rel}")
    get_filename_component(dest_dir "${dest}" DIRECTORY)
    add_custom_command(TARGET copy_lua PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dest_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy "${f}" "${dest}"
        COMMENT "Copying ${rel}"
    )
endforeach()
